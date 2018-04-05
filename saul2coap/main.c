/*
 * Copyright (C) 2018 HAW Hamburg
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
 * @brief       SAUL2CoAP main
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <stdio.h>

#include "fmt.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "phydat.h"
#include "saul_reg.h"
#include "shell.h"
#include "shell_commands.h"
#include "thread.h"
#include "xtimer.h"

#include "saul2coap.h"

int s2c_cmd(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "saul2coap", "Setup SAUL2CoAP background thread.", s2c_cmd },
    { NULL, NULL, NULL }
};

static char name[S2C_NAME_BUFLEN] = S2C_NODE_NAME;
static char path[S2C_PATH_BUFLEN];
static char json[S2C_JSON_BUFLEN];

static char saul2coap_stack[THREAD_STACKSIZE_MAIN];
static uint32_t s2c_interval = S2C_INTERVAL;
static volatile int run_loop = 0;

void *_saul2coap_loop(void *arg)
{
    (void)arg;

    xtimer_ticks32_t last_wakeup = xtimer_now();

    while (run_loop) {
        saul_reg_t *dev = saul_reg;

        if (dev == NULL) {
            puts("ERROR: No SAUL devices present");
            return NULL;
        }

        while (dev) {
            phydat_t res;
            if (dev->driver->type >= SAUL_SENSE_ANY) {
                int dim = saul_reg_read(dev, &res);
                if (dim < 1) {
                    continue;
                }

                memset(path, 0, S2C_PATH_BUFLEN);
                memset(json, 0, S2C_JSON_BUFLEN);

                int len = snprintf(path, S2C_PATH_BUFLEN, "/%s/%s/%s",
                                   name, dev->name,
                                   &saul_class_to_str(dev->driver->type)[6]);

                len = snprintf(json, S2C_JSON_BUFLEN, "{ ");
                for (int i = 0; i < dim; ++i) {
                    if (i > 0) {
                        len += snprintf(json + len, S2C_JSON_BUFLEN - len,
                                        ", ");
                    }
                    if (res.scale < 0) {
                        len += snprintf(json + len, S2C_JSON_BUFLEN - len,
                                        "\"v%i\": ", i);

                        len += fmt_s16_dfp(json + len, res.val[i], -res.scale);
                    }
                    else {
                        len += snprintf(json+len, S2C_JSON_BUFLEN - len,
                                        "\"v%i\": %"PRIi16, i, res.val[i]);
                        for (int i = 0; i < (int)res.scale; ++i) {
                            json[len++] = '0';
                        }
                    }
                }
                len += snprintf(json + len, S2C_JSON_BUFLEN - len, " }");
                printf("PATH: %s, JSON: %s\n", path, json);
                coap_post_sensor(path, json);
            }
            dev = dev->next;
            xtimer_sleep(1);
        }
        puts("********************************");

        xtimer_periodic_wakeup(&last_wakeup, s2c_interval);
    }
    return NULL;
}

void s2c_usage(void)
{
    puts("USAGE");
    puts("  saul2coap stop");
    puts("  saul2coap start IPADDR [PORT]");
    puts("    IPADDR    IP address of CoAP server");
    puts("    PORT      destination port of CoAP server");
    puts("  saul2coap interval [INTERVAL]");
    puts("    INTERVAL  CoAP send interval in seconds");
    puts("  saul2coap nodename [NAME]");
    puts("    NAME  Node name as string (maxlen=16)");
}

int s2c_cmd(int argc, char **argv)
{
    if ((argc == 2) && (strcmp(argv[1], "stop") == 0)) {
        run_loop = 0;
        return 0;
    }
    if ((argc == 2) && (strcmp(argv[1], "interval") == 0)) {
        printf("Interval = %"PRIu32"s\n", s2c_interval/US_PER_SEC);
        return 0;
    }
    if ((argc == 2) && (strcmp(argv[1], "nodename") == 0)) {
        printf("Nodename = %s\n", name);
        return 0;
    }
    if (argc > 2) {
        if (strcmp(argv[1], "start") == 0) {
            ipv6_addr_t addr;
            if (ipv6_addr_from_str(&addr, argv[2]) == NULL) {
                puts("ERROR: failed to parse server address!");
                return 2;
            }
            coap_server_addr(&addr);

            kernel_pid_t s2c_pid = thread_create(saul2coap_stack,
                                                 sizeof(saul2coap_stack),
                                                 THREAD_PRIORITY_MAIN - 1,
                                                 THREAD_CREATE_STACKTEST,
                                                 _saul2coap_loop, NULL, "s2c");
            if (s2c_pid > KERNEL_PID_UNDEF) {
                run_loop = 1;
                return 0;
            }
            puts("ERROR: failed to start SAUL2CoAP!");
            return 2;
        }
        else if (strcmp(argv[1], "interval") == 0) {
            long int tmp = strtol(argv[2], NULL, 10);
            if (tmp > 0) {
                s2c_interval = (uint32_t)tmp * US_PER_SEC;
                return 0;
            }
            puts("ERROR: invalid interval!");
            return 3;
        }
        else if (strcmp(argv[1], "nodename") == 0) {
            if (strlen(argv[2]) < S2C_NAME_BUFLEN) {
                strcpy(name, argv[2]);
                return 0;
            }
            puts("ERROR: invalid nodename!");
            return 4;
        }
    }
    s2c_usage();
    return 1;
}

int main(void)
{
    puts("\n*** SAUL-to-CoAP application ***\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
