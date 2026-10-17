/* Week 1 / Ex3 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

void periodic_task(void *argument);
void dwt_init(void);
void overhead_task(void *argument);
void overhead_report_task(void *argument);
void app_main(void)
{
    dwt_init();

    /* TODO: create periodic_task. Its priority must be the highest in the system
     *       — justify why in REPORT.md, and say what the measurement would look
     *       like if it were not. */

    BaseType_t overhead_created = xTaskCreate(
        overhead_task,
        "Overhead",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
    BaseType_t report_created = xTaskCreate(
        overhead_report_task,
        "UART Report",
        256,
        NULL,
        tskIDLE_PRIORITY + 2,
        NULL
    );

    configASSERT(overhead_created == pdPASS);
    configASSERT(report_created == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
