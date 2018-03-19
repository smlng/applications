/*
 * Copyright (c) 2017 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     vslab-riot
 * @{
 *
 * @file
 * @brief       CoAP wrapper code
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "msg.h"
#include "net/gcoap.h"
#include "net/ipv6/addr.h"

#include "saul2coap.h"

static const uint16_t s2c_port = S2C_COAP_SRV_PORT;
static const ipv6_addr_t s2c_addr = S2C_COAP_SRV_ADDR3;

static void _resp_handler(unsigned req_state, coap_pkt_t* pdu,
                          sock_udp_ep_t *remote)
{
    LOG_DEBUG("%s: begin\n", __func__);
    (void)remote;

    if (req_state == GCOAP_MEMO_TIMEOUT) {
        LOG_ERROR("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        LOG_ERROR("gcoap: error in response\n");
        return;
    }

    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
                            ? "Success" : "Error";
    LOG_INFO("gcoap: response %s, code %1u.%02u", class_str,
                                                coap_get_code_class(pdu),
                                                coap_get_code_detail(pdu));
    if (pdu->payload_len) {
        if ((pdu->content_type == COAP_FORMAT_TEXT) ||
            (pdu->content_type == COAP_FORMAT_LINK) ||
            (coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE) ||
            (coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE)) {
            /* Expecting diagnostic payload in failure cases */
            LOG_INFO("\n%.*s\n", pdu->payload_len, (char *)pdu->payload);
        }
        LOG_INFO(" with %u bytes\n", pdu->payload_len);
    }
    else {
        LOG_INFO("\n");
    }
    LOG_INFO("%s: done\n", __func__);
}

static size_t _send(const uint8_t *buf, size_t len)
{
    LOG_DEBUG("%s: begin\n", __func__);
    sock_udp_ep_t remote;

    remote.family   = AF_INET6;
    remote.netif    = SOCK_ADDR_ANY_NETIF;
    if (gnrc_netif_numof() == 1) {
        remote.netif = gnrc_netif_iter(NULL)->pid;
    }
    remote.port     = s2c_port;

    memcpy(&remote.addr.ipv6[0], &s2c_addr.u8[0], sizeof(s2c_addr.u8));
    LOG_DEBUG("%s: done\n", __func__);
    return gcoap_req_send2(buf, len, &remote, _resp_handler);
}

/* --- public coap interface --- */

int coap_post_sensor(char *path, const char *data)
{
    LOG_DEBUG("%s: begin\n", __func__);
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;

    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, path);

    len = strlen(data);
    memcpy(pdu.payload, data, len);
    pdu.payload[len++] = '\0';
    len = gcoap_finish(&pdu, len, COAP_FORMAT_TEXT);

    if (!_send(buf, len)) {
        LOG_ERROR("%s: send failed!\n", __func__);
        return 2;
    }
    LOG_DEBUG("%s: done\n", __func__);
    return 0;
}
