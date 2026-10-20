/* Week 1 / Ex4 — FreeRTOS hooks.
 *
 * Both hooks must be observable from outside the chip. A breakpoint is not a
 * deliverable: you need a UART line or a distinct LED pattern you can photograph.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include <string.h>
extern UART_HandleTypeDef huart2;
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* TODO 1: report xTask / pcTaskName out of the chip, then stop.
     *
     * Careful: the stack of the offending task is already corrupt. Think about
     * what you are allowed to call here, and write the reasoning in REPORT.md.
     * Hint: this hook runs from the scheduler's context, not the task's.
     */
    (void)xTask;
    const char tam[]= "STACK OVERFLOW, name: ";
    HAL_UART_Transmit(&huart2,(uint8_t*)tam,sizeof(tam)-1,100);
    if (pcTaskName !=NULL){
        HAL_UART_Transmit(&huart2,(uint8_t*)pcTaskName,strlen(pcTaskName),100);
    }
    const char skip[]= "\r\n";
        HAL_UART_Transmit(&huart2,(uint8_t*)skip,sizeof(skip)-1,100);
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    /* TODO 2: report, then stop. */
    const char tbao[] ="MALLOC FAILED";
    HAL_UART_Transmit(&huart2,(uint8_t*)tbao,sizeof(tbao)-1,100);
    for (;;) {
    }
}

/* configSUPPORT_STATIC_ALLOCATION = 1 makes these two callbacks mandatory.
 * Provide the memory for the idle task and the timer task here. */
static StaticTask_t xIdleTaskTCB;
static StackType_t  uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* TODO 3: static buffers. Note the size is in WORDS, not bytes —
     *         getting this wrong is a silent overflow. */
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
    
}
 static StaticTask_t xTimerTaskTCB;
 static StackType_t  uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ]; 
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* TODO 4: same, for the software timer task. */
    *ppxTimerTaskTCBBuffer =&xTimerTaskTCB;
    *ppxTimerTaskStackBuffer=uxTimerTaskStack;
    *pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;
}
