/* Week 1 / Ex4 — print the live RAM picture over USART2.
 *
 * This is your measurement instrument. It must be honest: read the values
 * from FreeRTOS, never from a constant you typed in.
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "ram_report.h"
#include "params.h"
#include "timers.h"

extern TaskHandle_t dyn_task_Handle;
extern TaskHandle_t stat_task_Handle;
extern size_t get_tcb_size_bytes(void);
extern UART_HandleTypeDef huart2;

/*
 * Blocking UART print helper.
 * Called from ram_report_task(), not from the stack overflow hook.
 */
void Uart_print(const char *data)
{
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)data,
                      (uint16_t)strlen(data),
                      100);
}

void ram_report_task(void *argument)
{
    (void)argument;

    char buffer[128];

    for (;;) {
        /* 1. Read live FreeRTOS heap measurements. */
        size_t free_heap = xPortGetFreeHeapSize();
        size_t min_ever_free_heap = xPortGetMinimumEverFreeHeapSize();

        Uart_print("\r\n--- FREERTOS RAM REPORT ---\r\n");

        snprintf(buffer, sizeof(buffer),
                 "HEAP free=%lu B, min_ever=%lu B\r\n",
                 (unsigned long)free_heap,
                 (unsigned long)min_ever_free_heap);
        Uart_print(buffer);

        snprintf(buffer, sizeof(buffer),
                 "Heap target >= %u B: %s\r\n",
                 (unsigned int)TARGET_FREE_HEAP_BYTES,
                 (free_heap >= TARGET_FREE_HEAP_BYTES) ? "PASS" : "FAIL");
        Uart_print(buffer);

        /* 2. Get task handles. NULL means the calling task. */
        TaskHandle_t tasks[] = {
            dyn_task_Handle,
            stat_task_Handle,
            xTaskGetCurrentTaskHandle(),
            xTaskGetIdleTaskHandle(),
            xTimerGetTimerDaemonTaskHandle()
        };

        const char *names[] = {
            "dyn",
            "stat",
            "ram_report",
            "idle",
            "timer"
        };

        /* 3. Print TCB size measured from the running firmware. */
        snprintf(buffer, sizeof(buffer),
                 "TCB size: %lu bytes\r\n",
                 (unsigned long)get_tcb_size_bytes());
        Uart_print(buffer);

        /* 4. Read and print HWM for every task.
         * HWM is in StackType_t words; convert to bytes using sizeof().
         */
        for (size_t i = 0; i < sizeof(tasks) / sizeof(tasks[0]); i++) {
            if (tasks[i] == NULL) {
                snprintf(buffer, sizeof(buffer),
                         "%s HWM: handle unavailable\r\n",
                         names[i]);
                Uart_print(buffer);
                continue;
            }

            UBaseType_t hwm = uxTaskGetStackHighWaterMark(tasks[i]);

            snprintf(buffer, sizeof(buffer),
                     "%s HWM: %lu words, %lu bytes\r\n",
                     names[i],
                     (unsigned long)hwm,
                     (unsigned long)(hwm * sizeof(StackType_t)));
            Uart_print(buffer);
        }

        /* 5. Wait before taking the next report. */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}