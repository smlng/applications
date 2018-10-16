/*
 * Copyright (c) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     applications
 * @{
 *
 * @file
 * @brief       fs1000a
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <stdio.h>

#include "irq.h"
#include "fmt.h"
#include "msg.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "periph/gpio.h"
#include "thread.h"
#include "xtimer.h"

#include "saul2coap.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

typedef struct {
    uint64_t values[2];
} fs1000a_sensor_data_t;

#define MAX(a,b)    ((a > b) ? a : b)
#define MIN(a,b)    ((a < b) ? a : b)

#define MAIN_QUEUE_SIZE     (8U)
#define FS1000A_PIN         GPIO_PIN(0, 0)
#define FS1000A_INTERVAL_MS (5000U)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static volatile uint32_t last = 0;
static volatile kernel_pid_t rt = KERNEL_PID_UNDEF;

static void _recv_cb(void *arg)
{
    (void)arg;

    uint32_t now = xtimer_now_usec();
    msg_t m;
    m.content.value = (uint32_t)(now - last);

    if (rt > KERNEL_PID_UNDEF) {
        msg_send(&m, rt);
    }
    last = now;
}

#define BUFLEN              (1700U)
#define DCBLEN              (16U)
#define SENSOR_THRESHOLD    (100000UL)

static uint32_t buffer[BUFLEN];
static uint64_t outbuf[DCBLEN];

static int _decode_plain2(uint32_t t1, uint32_t t2, size_t inpos,
                         const uint32_t *inbuf, size_t inlen,
                         uint64_t *outbuf, size_t outlen)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);

    uint64_t u64 = 0;
    unsigned last = 2;
    unsigned shift = 63;
    unsigned outpos = 0;
    for (size_t i = 0; i < inlen; ++i) {
        size_t pos = (inpos + i) % inlen;
        unsigned val = 0;
        if (inbuf[pos] > t1) {
            val = 1;
        }
        if (inbuf[pos] > t2) {
            val = 2;
            u64 = 0;
            shift = 63;
        }
        if ((val < 2) && (last < 2)) {
            if ((last == 1) && (val == 0)) {
                u64 |= 1ULL << shift;
            }
            last = 2;
            --shift;
        }
        else {
            last = val;
        }
        DEBUG("%d", val);

        if ((shift == 0) && (u64 > 0) &&
            (outbuf != NULL) && (outlen > 0) && (outpos < outlen)) {
            outbuf[outpos++] = u64;
            shift = 0;
            u64 = 0;
        }
    }
    DEBUG("\n");
    return outpos;
}

#define PATHLEN (24U)
#define JSONLEN (32U)
static char path[PATHLEN];
static char json[JSONLEN];

void send_data(fs1000a_sensor_data_t *sdat)
{
    int len = 0;
    /* send combined u64 for humidity and temperature */
    memset(path, 0, PATHLEN);
    memset(json, 0, JSONLEN);
    len = snprintf(path, PATHLEN, "/%s/%s/%s", S2C_NODE_NAME, "TFA","WIND");
    path[len] = '\0';
    printf("PATH: %s\n", path);
    len = snprintf(json, JSONLEN, "{ \"v0\": \"0x");
    len += fmt_u64_hex(json+len, sdat->values[0]);
    len += snprintf(json+len, JSONLEN-len, "\" }");
    printf("JSON: %s\n", json);
    coap_post_sensor(path, json);
    /* send u64 for wind speed */
    memset(path, 0, PATHLEN);
    memset(json, 0, JSONLEN);
    len = snprintf(path, PATHLEN, "/%s/%s/%s", S2C_NODE_NAME, "TFA","HUMTEMP");
    path[len] = '\0';
    printf("PATH: %s\n", path);
    len = snprintf(json, JSONLEN, "{ \"v0\": \"0x");
    len += fmt_u64_hex(json+len, sdat->values[1]);
    len += snprintf(json+len, JSONLEN-len, "\" }");
    printf("JSON: %s\n", json);
    coap_post_sensor(path, json);
}

static void print_ipv6_addresses(void)
{
    ipv6_addr_t ipv6_addrs[GNRC_NETIF_IPV6_ADDRS_NUMOF];
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);

    if (netif == NULL) {
        return;
    }

    kernel_pid_t iface = netif->pid;
    int res = gnrc_netapi_get(iface, NETOPT_IPV6_ADDR, 0,
                              ipv6_addrs, sizeof(ipv6_addrs));
    for (unsigned i = 0; i < (res / sizeof(ipv6_addr_t)); i++) {
        char addr_str[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_to_str(addr_str, &ipv6_addrs[i], sizeof(addr_str));
        printf(" IPv6 Address[%i]: %s\n", i, addr_str);
    }
}

int main(void)
{
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    rt = thread_getpid();

    xtimer_sleep(3);
    print_ipv6_addresses();
    xtimer_sleep(3);

    if (gpio_init_int(FS1000A_PIN, GPIO_IN, GPIO_BOTH, _recv_cb, NULL) < 0) {
        DEBUG("main: gpio_init_int failed!\n");
        return 1;
    }

    memset(buffer, 0, BUFLEN);
    unsigned counter = 0;

    while(1) {
        msg_t m;
        msg_receive(&m);

        uint32_t val = m.content.value;
        size_t pos = counter % BUFLEN;
        buffer[pos] = val;
        counter++;
        if (val > SENSOR_THRESHOLD) {
            memset(outbuf, 0, DCBLEN);
            unsigned ret = _decode_plain2(380, 580, pos, buffer, BUFLEN, outbuf, DCBLEN);
            if (ret > 0) {
                DEBUG("Decoded values U64 (%u):\n", ret);
                fs1000a_sensor_data_t sdat;
                unsigned pos = 0;
                for (unsigned i = 1; i <= ret; ++i) {
                    if ((pos == 0) || ((pos < 2) && (sdat.values[pos - 1] != outbuf[ret - i]))) {
                            sdat.values[pos++] = outbuf[ret - i];
                    }
                    print_u64_hex(outbuf[ret - i]);
                    puts("");
                }
                if (pos == 2) {
                    send_data(&sdat);
                }
                DEBUG("===================\n");
            }
        }


    }
    return 0;
}
