/*
 * Copyright (c) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    saul2coap
 * @brief       SAUL-to-CoAP example application
 * @{
 *
 * @file
 * @brief       CoAP interface
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef SAUL2COAP_H
#define SAUL2COAP_H

#include <stdbool.h>

#include "net/ipv6/addr.h"
#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef S2C_INTERVAL
#define S2C_INTERVAL        (30ULL * US_PER_SEC)
#endif

#ifndef S2C_NODE_NAME
#define S2C_NODE_NAME       ("SAUL2CoAP")
#endif

#define S2C_NAME_BUFLEN     (16U)
#define S2C_PATH_BUFLEN     (32U)
#define S2C_JSON_BUFLEN     (64U)

/**
 * @brief Set port of CoAP server
 *
 * @pre @p port must be > 0
 */
void coap_server_port(uint16_t port);

/**
 * @brief Set IP address of CoAP server
 *
 * @pre @p addr must not be unspecified or multicast
 */
void coap_server_addr(const ipv6_addr_t *addr);

/**
 * @brief Send (non-confirmable) CoAP POST request
 *
 * @param[in] path  resource handle
 * @param[in] data  message payload
 *
 * @returns 0 on success, or error otherwise
 */
int coap_post_sensor(char *path, const char *data);

#ifdef __cplusplus
}
#endif

#endif /* SAUL2COAP_H */
/** @} */
