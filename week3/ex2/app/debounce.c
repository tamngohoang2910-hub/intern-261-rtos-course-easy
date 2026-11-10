/* Week 3 / Ex2 -- debounce via one-shot timer + long-press via auto-reload
 * timer. Native FreeRTOS API only.
 */
#include <stdio.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "main.h"
#include "params.h"
#include "debounce.h"

static TimerHandle_t s_debounce_timer;
static TimerHandle_t s_longpress_timer;

static void debounce_callback(TimerHandle_t xTimer)
{
    (void)xTimer;

    /* TODO 1: printf/HAL_UART "PRESS" + xTaskGetTickCount(). This is the
     *         stable-press event, DEBOUNCE_MS after the last edge.
     *
     * TODO 2 (temporary, for the README step-4 experiment ONLY): add
     *         vTaskDelay(pdMS_TO_TICKS(50)) here, capture what happens to
     *         longpress_timer's callback timing in your REPORT.md, then
     *         DELETE this line again before you submit. A callback that
     *         blocks here is blocking the ONE daemon task every other
     *         software timer in the system also depends on. */
    GPIO_PinState s = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
    char buf[48];
    int n = snprintf(buf, sizeof(buf), "PRESS, tick=%lu\r\n",(unsigned long)xTaskGetTickCount());
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, n, 100);
    /*vTaskDelay(pdMS_TO_TICKS(50));*/
}

static void longpress_callback(TimerHandle_t xTimer)
{
    (void)xTimer;

    /* TODO 3: printf/HAL_UART "HELD" + xTaskGetTickCount(). Fires once per
     *         LONGPRESS_MS while the button stays down. */
    char tam[48];
    int a = snprintf(tam, sizeof(tam), "HELD, tick=%lu\r\n",
                     (unsigned long)xTaskGetTickCount());
    HAL_UART_Transmit(&huart1, (uint8_t *)tam, a, 100);
}

void debounce_init(void)
{
    /* TODO 4: s_debounce_timer = xTimerCreate(..., pdMS_TO_TICKS(DEBOUNCE_MS),
     *         pdFALSE, NULL, debounce_callback); -- pdFALSE = one-shot.
     *         s_longpress_timer = xTimerCreate(..., pdMS_TO_TICKS(LONGPRESS_MS),
     *         pdTRUE, NULL, longpress_callback); -- pdTRUE = auto-reload.
     *         Check both handles non-NULL. Do NOT xTimerStart() either here --
     *         they are driven entirely from debounce_on_edge(). */
    s_debounce_timer = xTimerCreate(
    "Debounce",
    pdMS_TO_TICKS(DEBOUNCE_MS),
    pdFALSE,
    NULL,
    debounce_callback
);

s_longpress_timer = xTimerCreate(
    "LongPress",
    pdMS_TO_TICKS(LONGPRESS_MS),
    pdTRUE,
    NULL,
    longpress_callback
);

if (s_debounce_timer == NULL || s_longpress_timer == NULL)
{
    const char msg[] = "ERROR: Timer creation failed!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg) - 1, 100);
    configASSERT(0);
}
}

void debounce_on_edge(uint8_t pin_is_high)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* TODO 5: xTimerResetFromISR(s_debounce_timer, &xHigherPriorityTaskWoken)
     *         on every edge -- this is what swallows bounce: each new edge
     *         restarts the DEBOUNCE_MS countdown, so the callback only fires
     *         once the line has been quiet for DEBOUNCE_MS. */

    /* TODO 6: if (pin_is_high) xTimerStartFromISR(s_longpress_timer, ...);
     *         else xTimerStopFromISR(s_longpress_timer, ...); */

    /* TODO 7: portYIELD_FROM_ISR(xHigherPriorityTaskWoken). */
     xTimerResetFromISR(
        s_debounce_timer,
        &xHigherPriorityTaskWoken
    );

       if (!pin_is_high)
    {
        xTimerStartFromISR(
            s_longpress_timer,
            &xHigherPriorityTaskWoken
        );
    }
    else
    {
        xTimerStopFromISR(
            s_longpress_timer,
            &xHigherPriorityTaskWoken
        );
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    (void)pin_is_high;
    (void)xHigherPriorityTaskWoken;
}
