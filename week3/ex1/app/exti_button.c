/* Week 3 / Ex1 -- EXTI button wakes a task via task notification.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "main.h"
#include "params.h"
#include "exti_button.h"
#include <string.h>

#define EXTI_PRIO_WRONG_TEST   0
static TaskHandle_t s_ack_task_handle;
extern UART_HandleTypeDef huart1;

static void assert_puts(const char *s)
{
    while (*s) {
        while ((huart1.Instance->SR & USART_SR_TXE) == 0) { }
        huart1.Instance->DR = (uint8_t)*s++;
    }
    while ((huart1.Instance->SR & USART_SR_TC) == 0) { }
}

void vAssertCalled(const char *file, int line)
{
    char msg[160];
    uint32_t preempt = 0, sub = 0;

    __disable_irq();

    HAL_NVIC_GetPriority(EXTI15_10_IRQn, NVIC_PRIORITYGROUP_4, &preempt, &sub);

    snprintf(msg, sizeof(msg),
             "\r\n=== FREERTOS ASSERT ===\r\n"
             "File: %s\r\n"
             "Line: %d\r\n"
             "EXTI Priority: %lu\r\n"
             "MAX SYSCALL PRIORITY: %d\r\n",
             file, line, (unsigned long)preempt,
             configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

    assert_puts(msg);

    for (;;) { }
}

void exti_button_init(void)
{
    /* TODO 1: GPIO init for PB<EXTI_PIN> as EXTI falling edge (GPIO_MODE_IT_FALLING,
     *         pull-up), and PA<ACK_PIN> as push-pull output.
     *
     * TODO 2 (do this FIRST, on purpose): HAL_NVIC_SetPriority(EXTIx_IRQn, <n>, 0)
     *         with <n> deliberately SMALLER (= higher hardware priority) than
     *         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY. Run step 4 of the
     *         README with this wrong value first and capture the configASSERT.
     *         Only after that, set it to ISR_NVIC_PRIO and HAL_NVIC_EnableIRQ. */
    #if EXTI_PRIO_WRONG_TEST
    HAL_NVIC_SetPriority(EXTI15_10_IRQn,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 1,
                         0);
#else
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, ISR_NVIC_PRIO, 0);
#endif

    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void exti_button_isr_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* TODO 3: vTaskNotifyGiveFromISR(s_ack_task_handle, &xHigherPriorityTaskWoken).
     *
     * TODO 4: portYIELD_FROM_ISR(xHigherPriorityTaskWoken) -- the README asks
     *         you to temporarily remove exactly this line in step 6 to measure
     *         the latency penalty. Do NOT remove it in the version you submit. */
    vTaskNotifyGiveFromISR(s_ack_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    (void)xHigherPriorityTaskWoken;
}

void ack_task(void *argument)
{
    (void)argument;
    s_ack_task_handle = xTaskGetCurrentTaskHandle();

    for (;;) {
        /* TODO 5: ulTaskNotifyTake(pdTRUE, portMAX_DELAY). */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        /* TODO 6: toggle PA<ACK_PIN> here, nothing before it in this loop body --
         *         any extra work here inflates the latency you are measuring. */
        GPIOA->ODR ^= (1U << ACK_PIN);
    }
}
