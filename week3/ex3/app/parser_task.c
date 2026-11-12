/* Week 3 / Ex3 -- command parser: rx_stream -> change demo_task's
 * period/priority/suspend state at runtime.
 *
 * Commands (each terminated by '\r', max MAX_CMD_LEN bytes):
 *   P<ms>\r   -- set demo_task period to <ms>
 *   S<prio>\r -- vTaskPrioritySet(demo_task, <prio>)
 *   T<0|1>\r  -- 0 = vTaskSuspend(demo_task), 1 = vTaskResume(demo_task)
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
#include <stdlib.h>

TaskHandle_t s_demo_task_handle;
static volatile uint32_t s_demo_period_ms = 200;
volatile uint32_t s_parser_cmd_count = 0;

static void apply_command(const char *cmd, size_t len)
{
    /* TODO 1: switch on cmd[0]: 'P' -> parse decimal into s_demo_period_ms
     *         (demo_task reads this every loop, no restart needed);
     *         'S' -> parse decimal, vTaskPrioritySet(s_demo_task_handle, n);
     *         'T' -> cmd[1]=='0' ? vTaskSuspend(...) : vTaskResume(...).
     *         Ignore anything malformed -- do not crash on bad input. */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
   if (cmd == NULL || len < 2 || s_demo_task_handle == NULL)
{
    return;
}

    /* Command P: Change demo period */
    if (cmd[0] == 'P')
    {
        char *end;
        unsigned long value;

        if (cmd[1] < '0' || cmd[1] > '9')
        {
            return;
        }

        value = strtoul(cmd + 1, &end, 10);

        if ((*end == '\0' || *end == '#') &&
            value >= 1UL &&          
            value <= 10000UL)
        {
            s_demo_period_ms = (uint32_t)value;
        }
    }

    /* Command S: Change task priority */
   else if (cmd[0] == 'S')
    {
        char *end;
        unsigned long value;

        if (cmd[1] < '0' || cmd[1] > '9')
        {
            return;
        }

        value = strtoul(cmd + 1, &end, 10);

        if ((*end == '\0' || *end == '#') &&
            value < (unsigned long)uxTaskPriorityGet(NULL))
        {
            unsigned long before = (unsigned long)uxTaskPriorityGet(s_demo_task_handle);   /* thêm */

            vTaskPrioritySet(s_demo_task_handle, (UBaseType_t)value);

            char msg[40];
            int  n = snprintf(msg, sizeof(msg), "S%lu OK prio: %lu -> %lu\r\n",
                              value,
                              before,                                                      /* trước */
                              (unsigned long)uxTaskPriorityGet(s_demo_task_handle));       /* sau */
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)n, 100);
        }
    }

    /* Command T: Suspend / Resume task */
    else if (cmd[0] == 'T')
{
    if (cmd[2] == '\0' || cmd[2] == '#')
    {   
        if (cmd[1] == '0')
        {
            vTaskSuspend(s_demo_task_handle);
        }
        else if (cmd[1] == '1')
        {
            vTaskResume(s_demo_task_handle);
        }
    }
}
    (void)cmd;
    (void)len;
}

void parser_task(void *argument)
{
    (void)argument;
    char cmd[MAX_CMD_LEN];
    size_t idx = 0;

    for (;;) {
        uint8_t byte;

        /* TODO 2: xStreamBufferReceive(rx_stream, &byte, 1, portMAX_DELAY). */

        /* TODO 3: if byte == '\r', call apply_command(cmd, idx), idx = 0.
         *         else if idx < MAX_CMD_LEN - 1, cmd[idx++] = byte.
         *         else (overflow): discard the whole partial command, idx = 0
         *         -- explain in REPORT.md why silently truncating instead
         *         would be worse. */

        xStreamBufferReceive(rx_stream,
                             &byte,
                             1,
                             portMAX_DELAY);
                             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        if (byte == '\n') {
        continue;
    }

        if (byte == '\r')
{
    if (idx > 0)
    {
        cmd[idx] = '\0';
        s_parser_cmd_count++;          
        apply_command(cmd, idx);
    }
    idx = 0;
}
        else if (idx < MAX_CMD_LEN - 1)
        {
            cmd[idx++] = (char)byte;
        }
        else
        {
            /* Overflow: discard the entire partial command */
            idx = 0;
        }
        (void)byte;
        (void)cmd;
        (void)idx;
    }
}

void demo_task(void *argument)
{
    (void)argument;
    s_demo_task_handle = xTaskGetCurrentTaskHandle();
    for (;;) {
        /* TODO 4: toggle a GPIO pin here. */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        vTaskDelay(pdMS_TO_TICKS(s_demo_period_ms));
    }
}
