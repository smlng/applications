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
#include "msg.h"
#include "periph/gpio.h"
#include "thread.h"
#include "xtimer.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

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

#define NUM_SAMPLES  64U
int main(void)
{
    rt = thread_getpid();
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    if (gpio_init_int(FS1000A_PIN, GPIO_IN, GPIO_BOTH, _recv_cb, NULL) < 0) {
        DEBUG("main: gpio_init_int failed!\n");
        return 1;
    }
    /* stats: max_now, max_all, min_now, min_all, avg_now, avg_all */
    //uint32_t samples[NUM_SAMPLES];
    uint32_t count, max, min, avg = 0;
    max = avg = count = 0;
    min = 0xffffffff;

    uint32_t now_ms, last_ms = (uint32_t)(xtimer_now_usec64() / US_PER_MS);
    now_ms = last_ms;

    while(++count) {
        if ((count % NUM_SAMPLES) == 0) {
            now_ms = (uint32_t)(xtimer_now_usec64() / US_PER_MS);
        }
        if ((now_ms - last_ms) > FS1000A_INTERVAL_MS) {
            //memset(samples, 0, NUM_SAMPLES);
            printf("%"PRIu32",%"PRIu32",%"PRIu32",%"PRIu32"\n", count, max, avg, min);
            max = 0;
            avg = 0;
            min = 0xffffffff;
        }
        msg_t m;
        msg_receive(&m);
        uint32_t val = m.content.value;
        //samples[count] = val;
        max = MAX(val, max);
        min = MIN(val, min);
        avg = (avg + val) / 2;
    }
    return 0;
}
