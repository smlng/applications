/*
 * Copyright (c) 2017 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    vslab-riot
 * @brief       VS practical exercise with RIOT-OS
 * @{
 *
 * @file
 * @brief       Leader Election
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

#ifndef S2C_COAP_SRV_PORT
#define S2C_COAP_SRV_PORT   (5683U)
#endif

#ifndef S2C_NODE_NAME
#define S2C_NODE_NAME       ("M02")
#endif

#ifndef S2C_PAN
#define S2C_PAN             (0x0604)
#endif

#ifndef S2C_CHN
#define S2C_CHN             (14U)
#endif

#ifndef S2C_TXP
#define S2C_TXP             (16U)
#endif

#define S2C_COAP_SRV_ADDR1  {{ 0xfd, 0x83, 0x20, 0x18, \
                               0x03, 0x23, 0x01, 0x03, \
                               0x00, 0x00, 0x00, 0x00, \
                               0x00, 0x00, 0x00, 0x01 }}

#define S2C_COAP_SRV_ADDR2  {{ 0xfd, 0x83, 0x20, 0x18, \
                               0x03, 0x23, 0x02, 0x03, \
                               0x00, 0x00, 0x00, 0x00, \
                               0x00, 0x00, 0x00, 0x01 }}

#define S2C_COAP_SRV_ADDR3  {{ 0xfd, 0x83, 0x20, 0x18, \
                               0x03, 0x23, 0x05, 0x03, \
                               0x00, 0x00, 0x00, 0x00, \
                               0x00, 0x00, 0x00, 0x01 }}
//fe80::1ac0:ffee:c0ff:ee24
#define S2C_COAP_SRV_ADDR4  {{ 0xfe, 0x80, 0x00, 0x00, \
                               0x00, 0x00, 0x00, 0x00, \
                               0x1a, 0xc0, 0xff, 0xee, \
                               0xc0, 0xff, 0xee, 0x24 }}
//fd16:abcd:ef24:3::1
#define S2C_COAP_SRV_ADDR5  {{ 0xfd, 0x16, 0xab, 0x0cd, \
                               0xef, 0x24, 0x00, 0x03, \
                               0x00, 0x00, 0x00, 0x00, \
                               0x00, 0x00, 0x00, 0x01 }}
//fe80::D0AA:2303:2018:0002
#define S2C_COAP_SRV_ADDR6  {{ 0xfe, 0x80, 0x00, 0x00, \
                               0x00, 0x00, 0x00, 0x00, \
                               0xd0, 0xaa, 0x23, 0x03, \
                               0x20, 0x18, 0x00, 0x02 }}


int coap_post_sensor(char *path, const char *data);

#ifdef __cplusplus
}
#endif

#endif /* SAUL2COAP_H */
/** @} */
