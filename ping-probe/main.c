/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    app_ping_probe
 * @brief       Small network probe answering pings and dumping received packets
 * @{
 *
 * @file
 * @brief       Application main loop
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <stdio.h>

#include "msg.h"
#include "net/gnrc.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/pktdump.h"

#define MAIN_QUEUE_SIZE     (8U)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static gnrc_netreg_entry_t dump_undef;

static void print_ipv6_addresses(void)
{
    puts("print_ipv6_addresses");
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
    /* message queue for potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("*** RIOT ping-probe ***");

    print_ipv6_addresses();

    gnrc_netreg_entry_init_pid(&dump_undef, GNRC_NETREG_DEMUX_CTX_ALL, gnrc_pktdump_pid);
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &dump_undef);

    /* should be never reached */
    return 0;
}
