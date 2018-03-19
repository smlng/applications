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

#include "fmt.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "xtimer.h"
#include "phydat.h"
#include "saul_reg.h"

#include "saul2coap.h"

/**
 * @brief   Read th sensors every second
 */
#define INTERVAL            (1LU * US_PER_SEC)


static int comm_init(void)
{
    uint16_t pan = S2C_PAN;
    uint16_t chn = S2C_CHN;
    uint16_t txp = S2C_TXP;
    uint16_t u16 = 0;
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
    return 0;
}

int main(void)
{
    if (comm_init() != 0) {
        return 1;
    }

    phydat_t res;
    xtimer_ticks32_t last_wakeup = xtimer_now();

    puts("\n*** SAUL-to-CoAP application ***\n");

    while (1) {
        saul_reg_t *dev = saul_reg;

        if (dev == NULL) {
            puts("No SAUL devices present");
            return 1;
        }

        while (dev) {
            if (dev->driver->type >= SAUL_SENSE_ANY) {
                int dim = saul_reg_read(dev, &res);
                if (dim < 1) {
                    continue;
                }

                char path[32] = {0};
                char json[64] = {0};

                int len = snprintf(path, 32, "/%s/%s/%s",
                                   S2C_NODE_NAME, dev->name,
                                   &saul_class_to_str(dev->driver->type)[6]);
                path[len] = '\0';
                printf("PATH: %s\n", path);
                len = snprintf(json, 64, "{ ");
                for (int i = 0; i < dim; ++i) {
                    if (i > 0) {
                        len += snprintf(json + len, 64,", ");
                    }
                    if (res.scale < 0) {
                    //
                        len += snprintf(json + len, 64,"\"v%i\": \"", i);

                        len += fmt_s16_dfp(json + len, res.val[i], -res.scale);
                        len += snprintf(json + len, 64,"\"");
                    }
                    /*
                    else if (res.scale > 0) {
                        len += snprintf(json+len, 64,"\"v%i\": \"%"PRIi16"E%i\"", i, res.val[i], (int)res.scale);
                    }*/
                    else {
                        len += snprintf(json+len, 64,"\"v%i\": \"%"PRIi16, i, res.val[i]);
                        for (int i = 0; i < (int)res.scale; ++i) {
                            json[len++] = '0';
                        }
                        len += snprintf(json+len, 64,"\"");
                    }
                }
                len += snprintf(json + len, 64," }");
                printf("JSON: %s\n", json);
                coap_post_sensor(path, json);
            }
            dev = dev->next;
        }
        puts("##########################");

        xtimer_periodic_wakeup(&last_wakeup, S2C_INTERVAL);
    }

    return 0;
}
