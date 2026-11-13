/* Week 3 / Ex4 -- CPU load, deadlock, watchdog.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "deadlock_demo.h"
#include "supervisor.h"
#define SUPERVISOR_PRIORITY    4
#define ALPHA_PRIORITY         3
#define BETA_PRIORITY          2
#define STATS_PRIORITY         1
void app_main(void)
{
    runtime_stats_timer_init();
    deadlock_demo_init();
    supervisor_init();

    /* TODO: xTaskCreate task_alpha, task_beta, supervisor_task,
     *       runtime_stats_task. Run deadlock_demo exactly as given first
     *       (step 2 of README) before touching task_alpha/task_beta. */
configASSERT(xTaskCreate(task_alpha, "Alpha", 128, NULL, ALPHA_PRIORITY,NULL) == pdPASS);


configASSERT(xTaskCreate(task_beta,"Beta",128,NULL,BETA_PRIORITY,NULL) == pdPASS);


configASSERT(xTaskCreate(supervisor_task,"Supervisor",128,NULL,SUPERVISOR_PRIORITY,NULL) == pdPASS);

configASSERT( xTaskCreate( runtime_stats_task,"RuntimeStats", 512, NULL, STATS_PRIORITY, NULL) == pdPASS);
MX_IWDG_Init();
    vTaskStartScheduler();
    for (;;) {
    }
}
