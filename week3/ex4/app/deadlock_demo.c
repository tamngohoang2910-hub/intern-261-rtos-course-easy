/* Week 3 / Ex4 -- task_alpha and task_beta share two resources, protected by
 * two mutexes, mimicking a sensor register + a shared UART buffer that both
 * tasks occasionally need at once. This file is provided complete, not a
 * skeleton: build and run it exactly as given first and use
 * uxTaskGetSystemState() to find out why it stops making progress, before
 * you change anything in here. Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdbool.h>
#include <stdio.h>
#include "main.h"
#include "params.h"
#include "deadlock_demo.h"
#include "supervisor.h"

static SemaphoreHandle_t s_mutex_a;
static SemaphoreHandle_t s_mutex_b;
volatile uint32_t alpha_max_dt = 0;   /* ngoài hàm, để in ở task khác */
volatile uint32_t alpha_sum_dt = 0;
volatile uint32_t alpha_cnt    = 0;
void deadlock_demo_init(void)
{
    s_mutex_a = xSemaphoreCreateMutex();
    s_mutex_b = xSemaphoreCreateMutex();
    configASSERT(s_mutex_a != NULL);
    configASSERT(s_mutex_b != NULL);
}

void task_alpha(void *argument)
{
    (void)argument;
    printf("START\r\n");
TickType_t last = 0;
    uint32_t loops = 0;
    bool slowed = false;
    for (;;) {
        xSemaphoreTake(s_mutex_a, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(5)); /* simulated register access time */
        xSemaphoreTake(s_mutex_b, portMAX_DELAY);

        /* ... shared work using both resources ... */
        xSemaphoreGive(s_mutex_b);
        xSemaphoreGive(s_mutex_a);

        /* TODO (step 4 of README, after the deadlock is fixed): call
         *       supervisor_checkin(SUPERVISOR_BIT_ALPHA) here, AFTER both
         *       mutexes are released -- that is what makes the check-in mean
         *       "this task completed a full cycle", not just "this task is
         *       still being scheduled". */
         if (!slowed && ++loops == 200) {
            vTaskDelay(pdMS_TO_TICKS(30));
            slowed = true;
        }
        TickType_t now = xTaskGetTickCount();
        if (last != 0) {
            uint32_t dt = (uint32_t)(now - last);
            if (dt > alpha_max_dt) alpha_max_dt = dt;
            alpha_sum_dt += dt;
            alpha_cnt++;
        }
        last = now;
         supervisor_checkin(SUPERVISOR_BIT_ALPHA);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void task_beta(void *argument)
{
    (void)argument;

    for (;;) {
        xSemaphoreTake(s_mutex_a, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(5)); /* simulated UART buffer access time */
        xSemaphoreTake(s_mutex_b, portMAX_DELAY);

        /* ... shared work using both resources ... */

        xSemaphoreGive(s_mutex_a);
        xSemaphoreGive(s_mutex_b);

        /* TODO (step 4 of README, after the deadlock is fixed): call
         *       supervisor_checkin(SUPERVISOR_BIT_BETA) here, AFTER both
         *       mutexes are released. */
        supervisor_checkin(SUPERVISOR_BIT_BETA);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
void deadlock_demo_report(void)
{
    TaskHandle_t ha = xSemaphoreGetMutexHolder(s_mutex_a);
    TaskHandle_t hb = xSemaphoreGetMutexHolder(s_mutex_b);
    printf("mutex_A held by: %s\r\n", ha ? pcTaskGetName(ha) : "none");
    printf("mutex_B held by: %s\r\n", hb ? pcTaskGetName(hb) : "none");
}