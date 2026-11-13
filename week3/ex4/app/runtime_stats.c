/* Week 3 / Ex4 -- run-time stats on DWT CYCCNT.
 * Native FreeRTOS API only.
 */
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "deadlock_demo.h".
extern volatile uint32_t alpha_max_dt;
extern volatile uint32_t alpha_sum_dt;
extern volatile uint32_t alpha_cnt;
void runtime_stats_timer_init(void)
{
    /* TODO 1: same DWT enable sequence as week1/ex2 dwt_init() (CoreDebug
     *         DEMCR TRCENA bit, zero DWT->CYCCNT, DWT->CTRL CYCCNTENA bit).
     *         If you already have week1/ex2's dwt_init(), the sequence is
     *         identical -- do not invent a second mechanism. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CYCCNT = 0;

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t runtime_stats_timer_value(void)
{
    /* TODO 2: return DWT->CYCCNT. This is portGET_RUN_TIME_COUNTER_VALUE()'s
     *         implementation -- wire it up in FreeRTOSConfig.deltas. */
    return DWT->CYCCNT;
}

void runtime_stats_task(void *argument)
{
    (void)argument;
    static char buf[512];
    uint32_t sample = 0;
    for (;;) {
        /* TODO 3: vTaskGetRunTimeStats(buf) and print buf over UART.
         *         Note: this walks every task's TCB -- it is itself not free,
         *         factor that into why STATS_WINDOW_MS should not be too
         *         short. */
        vTaskGetRunTimeStats(buf);
        sample++;
        printf("\r\n===== TASK RUNTIME STATS (lan do %lu) =====\r\n",
               (unsigned long)sample);
        printf("Task Name       Abs Time       CPU % %\r\n");
        printf("%s\r\n", buf);
        if (alpha_cnt) {
    printf("Alpha checkin: avg=%lu ms max=%lu ms (n=%lu)\r\n",
           (unsigned long)(alpha_sum_dt / alpha_cnt),
           (unsigned long)alpha_max_dt,
           (unsigned long)alpha_cnt);
}
        /*static TaskStatus_t st[8];
UBaseType_t n = uxTaskGetSystemState(st, 8, NULL);
static const char *const names[] = {"Running","Ready","Blocked","Suspended","Deleted","Invalid"};
printf("--- uxTaskGetSystemState ---\r\n");
for (UBaseType_t i = 0; i < n; i++) {
    printf("%-12s state=%s prio=%lu\r\n",
           st[i].pcTaskName,
           names[st[i].eCurrentState],
           (unsigned long)st[i].uxCurrentPriority);
}printf("--- mutex holders ---\r\n");
deadlock_demo_report();*/
        vTaskDelay(pdMS_TO_TICKS(STATS_WINDOW_MS));
    }
}
