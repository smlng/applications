/*
 * Copyright (c) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     saul2coap
 * @{
 *
 * @file
 * @brief       CoAP interface wrapper code
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"

#include "msg.h"
#include "net/gcoap.h"
#include "net/ipv6/addr.h"

#include "saul2coap.h"

static uint16_t s2c_port    = GCOAP_PORT;
static ipv6_addr_t s2c_addr = IPV6_ADDR_UNSPECIFIED;

static size_t _send(const uint8_t *buf, size_t len)
{
    assert(buf && len);

    sock_udp_ep_t remote;

    remote.family   = AF_INET6;
    remote.netif    = SOCK_ADDR_ANY_NETIF;
    if (gnrc_netif_numof() == 1) {
        remote.netif = gnrc_netif_iter(NULL)->pid;
    }
    remote.port     = s2c_port;

    memcpy(&remote.addr.ipv6[0], &s2c_addr.u8[0], sizeof(s2c_addr.u8));

    return gcoap_req_send2(buf, len, &remote, NULL);
}

/* --- public coap interface --- */
void coap_server_port(uint16_t port)
{
    assert(port);

    s2c_port = port;
}

void coap_server_addr(const ipv6_addr_t *addr)
{
    assert(addr);

    memcpy(&s2c_addr, addr, sizeof(ipv6_addr_t));
}

int coap_post_sensor(char *path, const char *data)
{
    if (ipv6_addr_is_unspecified(&s2c_addr) || ipv6_addr_is_multicast(&s2c_addr)) {
        puts("ERROR: invalid CoAP server address!");
        return 1;
    }

    if (!path || !data) {
        puts("ERROR: invalid path or data!");
        return 2;
    }

    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;

    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, path);
    coap_hdr_set_type(pdu.hdr, COAP_TYPE_NON);
    len = strlen(data);
    memcpy(pdu.payload, data, len);
    len = gcoap_finish(&pdu, len, COAP_FORMAT_JSON);

    if (!_send(buf, len)) {
        puts("ERROR: coap send failed!");
        return 2;
    }

    return 0;
}
