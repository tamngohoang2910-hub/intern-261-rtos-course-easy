
/* Week 2 / Ex1 — sensor pipeline:
 * sensor_task -> queue_raw -> filter_task ->
 * queue_filtered -> uart_task.
 *
 * Native FreeRTOS API only.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

#include <stdio.h>
#include <string.h>

#define LOG_N 100

QueueHandle_t queue_raw;
QueueHandle_t queue_filtered;

/* Giữ nguyên các biến của Code 2 */
volatile UBaseType_t depth_raw_now = 0U;
volatile UBaseType_t depth_filt_now = 0U;

/* RAM buffer for queue-depth measurements */
typedef struct {
    uint16_t tick;
    uint8_t raw;
    uint8_t filt;
} depth_t;

static depth_t depth_log[LOG_N];

/* Task handles */
TaskHandle_t h_sensor;
TaskHandle_t h_filter;
TaskHandle_t h_uart;

/* Error handler */
static void error_fatal(const char *msg)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)msg,
        strlen(msg),
        HAL_MAX_DELAY
    );

    for (;;) {}
}

/* Depth logger task */
void depth_logger_task(void *argument)
{
    (void)argument;

    TickType_t last_wake = xTaskGetTickCount();

    /* B1: Sample queue depth every 1 ms */
    for (uint32_t i = 0; i < LOG_N; i++) {

        depth_raw_now = uxQueueMessagesWaiting(queue_raw);
        depth_filt_now = uxQueueMessagesWaiting(queue_filtered);

        depth_log[i].tick = (uint16_t)xTaskGetTickCount();
        depth_log[i].raw = (uint8_t)depth_raw_now;
        depth_log[i].filt = (uint8_t)depth_filt_now;

        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(1U)
        );
    }

    /* B2: Stop sensor task */
    vTaskSuspend(h_sensor);

    /* Wait until pipeline is completely drained */
    while (
        uxQueueMessagesWaiting(queue_raw) != 0 ||
        uxQueueMessagesWaiting(queue_filtered) != 0 ||
        eTaskGetState(h_filter) != eBlocked ||
        eTaskGetState(h_uart) != eBlocked
    ) {
        vTaskDelay(pdMS_TO_TICKS(10U));
    }

    /* B3: Export measurement data through UART */
    char line[32];

    for (uint32_t i = 0; i < LOG_N; i++) {

        int n = snprintf(
            line,
            sizeof(line),
            "D,%u,%u,%u\r\n",
            (unsigned)depth_log[i].tick,
            (unsigned)depth_log[i].raw,
            (unsigned)depth_log[i].filt
        );

        if (n > 0) {
            HAL_UART_Transmit(
                &huart1,
                (uint8_t *)line,
                (uint16_t)n,
                HAL_MAX_DELAY
            );
        }
    }

    /* Stop logger task */
    vTaskSuspend(NULL);
}

/* Application entry */
void app_main(void)
{
    /* TODO 1: Create queues */

    queue_raw = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(raw_sample_t)
    );

    queue_filtered = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(filtered_sample_t)
    );

    if (queue_raw == NULL || queue_filtered == NULL) {
        error_fatal("Queue creation failed\r\n");
    }

    /* TODO 2: Create pipeline tasks */

    if (xTaskCreate(
            sensor_task,
            "sensor",
            256,
            NULL,
            3,
            &h_sensor
        ) != pdPASS) {
        error_fatal("Sensor task creation failed\r\n");
    }

    if (xTaskCreate(
            filter_task,
            "filter",
            256,
            NULL,
            1,
            &h_filter
        ) != pdPASS) {
        error_fatal("Filter task creation failed\r\n");
    }

    if (xTaskCreate(
            uart_task,
            "uart",
            256,
            NULL,
            1,
            &h_uart
        ) != pdPASS) {
        error_fatal("UART task creation failed\r\n");
    }

    /* TODO 3: Create depth logger task */

    if (xTaskCreate(
            depth_logger_task,
            "depth_logger",
            512,
            NULL,
            4,
            NULL
        ) != pdPASS) {
        error_fatal("Logger task creation failed\r\n");
    }

    vTaskStartScheduler();

    for (;;) {}
}
