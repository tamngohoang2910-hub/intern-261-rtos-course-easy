/* Week 2 / Ex2 — four ways to serialize access to USART1 from WRITER_COUNT
 * tasks. Only one UART_GUARD_MODE is compiled at a time.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


#include "main.h"
#include "params.h"
#include "uart_guard.h"

#if UART_GUARD_MODE == 1
static SemaphoreHandle_t s_uart_mutex;
#elif UART_GUARD_MODE == 3
static QueueHandle_t s_gatekeeper_queue;
#endif
void writer_task(void *argument);
void uart_guard_init(void)
{
#if UART_GUARD_MODE == 1
    /* TODO 1: xSemaphoreCreateMutex(). Check for NULL. */
    s_uart_mutex = xSemaphoreCreateMutex();

    if (s_uart_mutex == NULL) {
        Error_Handler();
    }

#elif UART_GUARD_MODE == 3
    /* TODO 2: create s_gatekeeper_queue (length: pick and justify in
     *         REPORT.md), item size = MSG_LEN bytes.
     *         xTaskCreate(gatekeeper_task, ..., GATEKEEPER_PRIO). */
    s_gatekeeper_queue = xQueueCreate(
    5,          /* Queue length: 5 messages */
    MSG_LEN     /* Size of each message: MSG_LEN bytes */
);configASSERT(s_gatekeeper_queue != NULL);

BaseType_t result = xTaskCreate(
    gatekeeper_task,
    "Gatekeeper",
    256,
    NULL,
    GATEKEEPER_PRIO,
    NULL
);

configASSERT(result == pdPASS);
#endif
}

void uart_guard_send(uint32_t id, const char *buf, size_t len)
{
#if UART_GUARD_MODE == 0
    /* The bug: no serialization at all. */
    /* TODO 3: HAL_UART_Transmit directly. */
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
    (void)buf; (void)len;

#elif UART_GUARD_MODE == 1
    /* TODO 4: xSemaphoreTake(s_uart_mutex, portMAX_DELAY), transmit, Give.
     *         Every return path must Give — no early return between Take and
     *         Give. */
    if (id == 0) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    }
    if (xSemaphoreTake(s_uart_mutex, portMAX_DELAY) == pdTRUE)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    xSemaphoreGive(s_uart_mutex);
}
    (void)buf; (void)len;

#elif UART_GUARD_MODE == 2
    /* TODO 5: taskENTER_CRITICAL(), transmit, taskEXIT_CRITICAL().
     *         Measure in REPORT.md how long the critical section holds IRQs
     *         off, as a function of MSG_LEN and UART_BAUD. */
     if (id == 0) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    }
    taskENTER_CRITICAL();

    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    taskEXIT_CRITICAL();
    (void)buf; (void)len;
#elif UART_GUARD_MODE == 3
    /* TODO 6: xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY).
     *         The writer never touches UART directly in this mode. */
    if (id == 0) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    }
    if (xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY) != pdPASS) {
    Error_Handler();
    }
    (void)buf; (void)len;
#endif
}

#if UART_GUARD_MODE == 3
void gatekeeper_task(void *argument)
{
    (void)argument;
    for (;;) {
        char msg[MSG_LEN];
        /* TODO 7: xQueueReceive(s_gatekeeper_queue, msg, portMAX_DELAY), then
         *         HAL_UART_Transmit. This is the only task allowed to call
         *         HAL_UART_Transmit in this mode. */
        if (xQueueReceive(s_gatekeeper_queue, msg, portMAX_DELAY) == pdPASS) {
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, MSG_LEN, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        (void)msg;
        }
    }
}
#endif
