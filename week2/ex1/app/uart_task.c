
/* Week 2 / Ex1 — prints filtered samples to USART1 at UART_BAUD.
 * queue_filtered -> UART.
 *
 * This is also the task you deliberately slow down in step 7 by setting
 * the UART hardware baud rate to 9600 to observe queue_filtered filling.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>

#include "main.h"
#include "params.h"
#include "pipeline.h"

extern UART_HandleTypeDef huart1;

void uart_task(void *argument)
{
    (void)argument;

    /* Count filtered samples sent since the last queue-depth report. */
    uint32_t depth_print_cnt = 0U;

    for (;;) {
        filtered_sample_t in;

        /* Wait until a filtered sample is available. */
        if (xQueueReceive(queue_filtered, &in, portMAX_DELAY) != pdPASS) {
            continue;
        }

        /* Format and transmit: seq,ax,ay,az */
        char line[40];

        int len = snprintf(line, sizeof(line),
                           "%lu,%d,%d,%d\r\n",
                           (unsigned long)in.seq,
                           (int)in.ax,
                           (int)in.ay,
                           (int)in.az);

        if (len > 0 && len < (int)sizeof(line)) {
            (void)HAL_UART_Transmit(&huart1,
                                    (uint8_t *)line,
                                    (uint16_t)len,
                                    20U);
        }

        /*
         * Report queue depths every 250 received samples.
         * Note: this extra UART output adds transmission overhead and can
         * affect queue behavior. Remove this block for the least intrusive
         * measurement, or inspect depth_raw_now/depth_filt_now in the debugger.
         */
        depth_print_cnt++;

        if (depth_print_cnt >= 25U) {
            depth_print_cnt = 0U;

            char dline[40];

            int dlen = snprintf(dline, sizeof(dline),
                    "# raw=%u filt=%u drop=%lu\r\n",
                    (unsigned)depth_raw_now,
                    (unsigned)depth_filt_now,
                    (unsigned long)g_filtered_drop_cnt);
            if (dlen > 0 && dlen < (int)sizeof(dline)) {
                (void)HAL_UART_Transmit(&huart1,
                                        (uint8_t *)dline,
                                        (uint16_t)dlen,
                                        20U);
            }
        }
    }
}
