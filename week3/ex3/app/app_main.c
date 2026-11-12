/* Week 3 / Ex3 -- UART RX -> stream buffer -> parser.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"

#define PARSER_TASK_STACK_SIZE    256
#define DEMO_TASK_STACK_SIZE      256

#define PARSER_TASK_PRIORITY      (tskIDLE_PRIORITY + 3)
#define DEMO_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)
void app_main(void)
{

    uart_rx_init();
    /* TODO: xTaskCreate parser_task and demo_task. parser_task must run at a
     *       priority high enough that it drains rx_stream faster than bytes
     *       arrive at RX_BAUD -- justify the number in REPORT.md with the
     *       byte time you computed. */
    BaseType_t parser_status = xTaskCreate(
        parser_task,
        "Parser",
        PARSER_TASK_STACK_SIZE,
        NULL,
        PARSER_TASK_PRIORITY,
        NULL
    );

    configASSERT(parser_status == pdPASS);

    /* Create demo task */
    BaseType_t demo_status = xTaskCreate(
        demo_task,
        "Demo",
        DEMO_TASK_STACK_SIZE,
        NULL,
        DEMO_TASK_PRIORITY,
        &s_demo_task_handle
    );

    configASSERT(demo_status == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
