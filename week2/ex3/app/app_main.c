/* Week 2 / Ex3 — priority inversion.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "dwt.h"
#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

#define PRIO_LOW    1
#define PRIO_MID    2
#define PRIO_HIGH   3

TaskHandle_t task_low_handle;
TaskHandle_t task_mid_handle;
TaskHandle_t task_high_handle;
void app_main( void )
{
    BaseType_t xResult;

    DWT_Init();             
    resource_init();       
    xResult = xTaskCreate( task_low, "LOW", 256, NULL, PRIO_LOW, &task_low_handle );
    configASSERT( xResult == pdPASS );

    xResult = xTaskCreate( task_mid, "MID", 256, NULL,PRIO_MID, &task_mid_handle );
    configASSERT( xResult == pdPASS );

    xResult = xTaskCreate( task_high, "HIGH", 256, NULL, PRIO_HIGH, &task_high_handle );
    configASSERT( xResult == pdPASS );
    ( void ) xResult;
    vTaskStartScheduler();
    configASSERT( 0 );
    for( ;; )
    {
    }
}
