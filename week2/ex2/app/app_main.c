/* Week 2 / Ex2 — race condition & mutex.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"
#include <stdio.h> 
void writer_task(void *argument);
/*static void monitor_task(void *argument)
{
    (void)argument;

    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(5);

    for (;;) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_10);

        vTaskDelayUntil(&last_wake, period);
    }
}*/
void app_main(void)
{
    uart_guard_init();

    /* TODO: xTaskCreate WRITER_COUNT instances of writer_task, each given its
     *       own index (0..WRITER_COUNT-1) as the task parameter.
     *       Do not create WRITER_COUNT separately-named functions. */
    for (uint32_t i = 0; i < WRITER_COUNT; i++){
    char task_name[16];
    snprintf(task_name, sizeof(task_name), "Writer%lu", (unsigned long)i);
    BaseType_t result=xTaskCreate(
        writer_task,
        task_name,
        256,
        (void *)(uintptr_t)i,
        tskIDLE_PRIORITY+ 1,
        NULL
    );
configASSERT(result == pdPASS);
}
  /*BaseType_t moni = xTaskCreate(
    monitor_task,
    "Monitor",
    128,
    NULL,
    tskIDLE_PRIORITY + 2,
    NULL
);
configASSERT(moni == pdPASS);
*/
    vTaskStartScheduler();

    for (;;) {
    }
}
