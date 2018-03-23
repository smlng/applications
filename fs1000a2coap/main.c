/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 *
 * @file
 * @brief       Test the SAUL interface of devices by periodically reading from
 *              them
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 */

#include <stdio.h>

#define FS1000A_PARAM_RECV_PIN      GPIO_PIN(0, 28)

#include "fs1000a.h"
#include "fs1000a_params.h"
#include "thread.h"
#include "xtimer.h"
#include "fmt.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"

#include "saul2coap.h"

static int comm_init(void)
{
    uint16_t pan = S2C_PAN;
    uint16_t chn = S2C_CHN;
    uint16_t txp = S2C_TXP;
    uint16_t u16 = 0;
    ipv6_addr_t ipv6_addrs[GNRC_NETIF_IPV6_ADDRS_NUMOF];
    /* get the PID of the first radio */
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);

    if (netif == NULL) {
        puts("!! comm_init failed, no network interface found !!");
        return 1;
    }
    else {
        puts ("*** Primary Network Interface Configuration ***");
    }
    kernel_pid_t iface = netif->pid;
    /* initialize the radio */
    if (gnrc_netapi_set(iface, NETOPT_NID, 0, &pan, 2) < 0) {
        puts("!! failed to set PAN ID !!");
    }
    else if (gnrc_netapi_get(iface, NETOPT_NID, 0, &u16, sizeof(u16)) >= 0) {
            printf(" PAN ID: 0x%" PRIx16"\n", u16);
    }
    if (gnrc_netapi_set(iface, NETOPT_CHANNEL, 0, &chn, 2) < 0) {
        puts("!! failed to set channel !!");
    }
    else if (gnrc_netapi_get(iface, NETOPT_CHANNEL, 0, &u16, sizeof(u16)) >= 0) {
            printf(" Channel: %" PRIu16 "\n", u16);
    }
    if (gnrc_netapi_set(iface, NETOPT_TX_POWER, 0, &txp, 2) < 0) {
        puts("!! failed to tx power !!");
    }
    else if (gnrc_netapi_get(iface, NETOPT_TX_POWER, 0, &u16, sizeof(u16)) >= 0) {
            printf(" TX Power: %" PRIu16 "\n", u16);
    }
    int res = gnrc_netapi_get(iface, NETOPT_IPV6_ADDR, 0, ipv6_addrs, sizeof(ipv6_addrs));
    for (unsigned i = 0; i < (res / sizeof(ipv6_addr_t)); i++) {
        char addr_str[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_to_str(addr_str, &ipv6_addrs[i], sizeof(addr_str));
        printf(" IPv6 Address[%i]: %s\n", i, addr_str);
    }
    return 0;
}

void print_ipv6(void)
{
    ipv6_addr_t ipv6_addrs[GNRC_NETIF_IPV6_ADDRS_NUMOF];
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);

    if (netif == NULL) {
        return;
    }

    kernel_pid_t iface = netif->pid;
    int res = gnrc_netapi_get(iface, NETOPT_IPV6_ADDR, 0, ipv6_addrs, sizeof(ipv6_addrs));
    for (unsigned i = 0; i < (res / sizeof(ipv6_addr_t)); i++) {
        char addr_str[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_to_str(addr_str, &ipv6_addrs[i], sizeof(addr_str));
        printf(" IPv6 Address[%i]: %s\n", i, addr_str);
    }
}

#define PATHLEN (32U)
#define JSONLEN (64U)

int main(void)
{
    if (comm_init() != 0) {
        return 1;
    }
    xtimer_sleep(32);
    puts("");
    print_ipv6();
    puts("");

    puts("\n*** FS1000A-to-CoAP application ***\n");

    fs1000a_t dev;

    puts("Init FS1000A");
    if (fs1000a_init(&dev, &fs1000a_params[0])) {
        puts("[FAILED]");
        return 1;
    }

    fs1000a_enable_sensor_receive(&dev, thread_getpid());

    while(1) {
        msg_t m;
        msg_receive(&m);
        fs1000a_sensor_data_t *sdat = (fs1000a_sensor_data_t *)m.content.ptr;
        char path[PATHLEN] = {0};
        char json[JSONLEN] = {0};

        int len = snprintf(path, PATHLEN, "/%s/%s/%s", S2C_NODE_NAME, "TFA-THW","RAW");
        path[len] = '\0';
        printf("PATH: %s\n", path);
        len = snprintf(json, JSONLEN, "{ \"v0\": \"0x");
        len += fmt_u64_hex(json+len, sdat->values[0]);
        len += snprintf(json+len, JSONLEN-len, "\", \"v1\": \"0x");
        len += fmt_u64_hex(json+len, sdat->values[1]);
        len += snprintf(json+len, JSONLEN-len, "\" }");
        printf("JSON: %s\n", json);
        coap_post_sensor(path, json);
    }

    return 0;
}
