/* Week 2 / Ex4 — event group sync across EVENT_BIT_COUNT producers, with
 * producer_0's signal optionally replaced by a task notification.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include <stdio.h>
#include "main.h"
#include "params.h"
#include "sync_tasks.h"

static EventGroupHandle_t s_events;
static volatile uint32_t t0;
static volatile uint32_t t0_n;
void *sync_task_handle;
#define TEST_SLOW_PRODUCER 0
#define ALL_BITS_MASK ((1u << EVENT_BIT_COUNT) - 1u)
extern UART_HandleTypeDef huart2;

int _write(int file, char *ptr, int len)
{
    (void)file;

    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)ptr,
        len,
        HAL_MAX_DELAY
    );

    return len;
}
void sync_init(void)
{
    size_t heap_before = xPortGetFreeHeapSize();     
    /* TODO 1: xEventGroupCreate(). Check for NULL.
     *         Measure and record in REPORT.md the sizeof the structure this
     *         allocates (see event_groups.c in the kernel sources) to answer
     *         PREDICTION.md question 1. */
    s_events = xEventGroupCreate();
    size_t heap_after = xPortGetFreeHeapSize(); 
    if (s_events == NULL)
    {
        Error_Handler();
    }
    printf("EG heap = %lu bytes, sizeof(StaticEventGroup_t) = %lu bytes\r\n",
           (unsigned long)(heap_before - heap_after),
           (unsigned long)sizeof(StaticEventGroup_t));

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

void producer_task(void *argument)
{
    uint32_t idx = (uint32_t)(uintptr_t)argument;
    uint32_t rand_value = 0x9E3779B9u * (idx + 1u);
    for (;;) {
        /* TODO 2: random small period (reuse a simple xorshift like ex3's
         *         periodic task, or vTaskDelay with a bounded random jitter). */
        rand_value ^= rand_value << 13; 
        rand_value ^= rand_value >> 17;
        rand_value ^= rand_value << 5;
        uint32_t delay_ms = 10 + (rand_value % 21); 
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
#if TEST_SLOW_PRODUCER
        if (idx == 3) {
            vTaskDelay(pdMS_TO_TICKS(SYNC_TIMEOUT_MS + 50));  
        }
#endif

#if PRODUCER0_USE_NOTIFY
        if (idx == 0) {
            /* TODO 3: xTaskNotifyGive((TaskHandle_t)sync_task_handle).
             *         Toggle a DWT-timestamped GPIO edge right before this
             *         call — this is your t0 for the wake-up latency
             *         measurement. */
            t0 = DWT->CYCCNT;
            t0_n = t0;  
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
            xTaskNotifyGive((TaskHandle_t)sync_task_handle);
        } else
#endif
        {
            /* TODO 4: xEventGroupSetBits(s_events, 1u << idx).
             *         Same timestamp discipline as TODO 3 for producers that
             *         still use the event group. */
            t0 = DWT->CYCCNT;
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
            xEventGroupSetBits(s_events, 1u << idx);
        }
    }
}

void sync_task(void *argument)
{
    (void)argument;
    uint32_t sync_count = 0;
    TickType_t last_sync_tick = 0;
    
    for (;;) {
        uint32_t sync_success = 0;
        uint32_t t1 = 0; 
        EventBits_t bits = 0;
        uint32_t noti_lat = 0;
        uint32_t noti_valid = 0;
#if PRODUCER0_USE_NOTIFY
        /* TODO 5: ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SYNC_TIMEOUT_MS)) for
         *         producer_0's contribution, THEN xEventGroupWaitBits for the
         *         remaining producers' bits (mask without bit 0), clear-on-exit
         *         = pdTRUE, wait-for-all = pdTRUE, same timeout.
         *         Toggle a GPIO edge the moment both are satisfied — your t1
         *         for the latency measurement. */
        const EventBits_t rest_mask = ALL_BITS_MASK & ~1u;

        uint32_t got0 = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SYNC_TIMEOUT_MS));
        uint32_t t1n = DWT->CYCCNT;                        
        if (got0 != 0u) {
            noti_lat = t1n - t0_n;
            noti_valid = 1u;
        }
        bits = xEventGroupWaitBits(
            s_events,
            rest_mask,
            pdTRUE,
            pdTRUE,
            pdMS_TO_TICKS(SYNC_TIMEOUT_MS)
        );

        if ((got0 != 0u) && ((bits & rest_mask) == rest_mask)) {
            sync_success = 1;
            t1 = DWT->CYCCNT;                          
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
        } 
#else
        /* TODO 6: xEventGroupWaitBits(s_events, ALL_BITS_MASK, pdTRUE, pdTRUE,
         *         pdMS_TO_TICKS(SYNC_TIMEOUT_MS)).
         *         Check the return value against ALL_BITS_MASK to tell
         *         "all bits arrived" apart from "timed out with some bits
         *         missing" — do not assume a non-zero return means success. */
        bits = xEventGroupWaitBits(
        s_events,
        ALL_BITS_MASK,
        pdTRUE,
        pdTRUE,
        pdMS_TO_TICKS(SYNC_TIMEOUT_MS)
    );

    if ((bits & ALL_BITS_MASK) == ALL_BITS_MASK) {
        sync_success = 1;                              
        t1 = DWT->CYCCNT;                    
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15); 
        }
#endif

        /* TODO 7: print the sync instant and measured period to UART. */
        TickType_t now = xTaskGetTickCount();

if (sync_success) {
    uint32_t period_ms = 0;

    if (sync_count > 0) {
        period_ms = (now - last_sync_tick) *
                    portTICK_PERIOD_MS;
    }

    sync_count++;
    last_sync_tick = now;

    printf("SYNC #%lu: t=%lu ms, period=%lu ms, lat=%lu cycles\r\n",   /* [SỬA] thêm lat */
           (unsigned long)sync_count,
           (unsigned long)(now * portTICK_PERIOD_MS),
           (unsigned long)period_ms,
           (unsigned long)(t1 - t0));
           if (noti_valid) {
        printf("NOTI lat=%lu cycles\r\n", (unsigned long)noti_lat);
    }
} else {
    printf("SYNC TIMEOUT: ret=0x%lX, mask=0x%lX, thieu=0x%lX\r\n",
           (unsigned long)bits,
           (unsigned long)ALL_BITS_MASK,
           (unsigned long)(ALL_BITS_MASK & ~bits));
}
    }
}
