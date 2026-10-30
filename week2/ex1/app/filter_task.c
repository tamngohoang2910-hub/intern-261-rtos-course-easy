
/* Week 2 / Ex1 — moving-average filter, window = FILTER_WINDOW samples.
 * queue_raw -> queue_filtered.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"
volatile uint32_t g_filtered_drop_cnt = 0U;
void filter_task(void *argument)
{
    (void)argument;

    /* B1: Initialize ring buffers and running sums */

    static int16_t buf_ax[FILTER_WINDOW] = {0};
    static int16_t buf_ay[FILTER_WINDOW] = {0};
    static int16_t buf_az[FILTER_WINDOW] = {0};
    
    int32_t sum_ax = 0;
    int32_t sum_ay = 0;
    int32_t sum_az = 0;

    uint32_t idx = 0U;
    uint32_t count = 0U;

    for (;;)
    {
        raw_sample_t in;

        /* B2: Receive raw sample from queue_raw */

        if (xQueueReceive(queue_raw, &in, portMAX_DELAY) != pdPASS)
        {
            continue;
        }

        /* B3: Update running sums */

        sum_ax += (int32_t)in.ax - (int32_t)buf_ax[idx];
        sum_ay += (int32_t)in.ay - (int32_t)buf_ay[idx];
        sum_az += (int32_t)in.az - (int32_t)buf_az[idx];

        /* B4: Update ring buffer */

        buf_ax[idx] = in.ax;
        buf_ay[idx] = in.ay;
        buf_az[idx] = in.az;

        /* B5: Move index to next position */

        idx++;

        if (idx >= FILTER_WINDOW)
        {
            idx = 0U;
        }

        /* B6: Count valid samples */

        if (count < FILTER_WINDOW)
        {
            count++;
        }

        /* Wait until the window is full */

        if (count < FILTER_WINDOW)
        {
            continue;
        }

        /* B7: Calculate moving average */

        filtered_sample_t out;

        out.seq = in.seq;

        out.ax = (int16_t)(sum_ax / (int32_t)FILTER_WINDOW);
        out.ay = (int16_t)(sum_ay / (int32_t)FILTER_WINDOW);
        out.az = (int16_t)(sum_az / (int32_t)FILTER_WINDOW);

        /* B8: Send filtered sample without blocking */

        if (xQueueSend(queue_filtered, &out, 0) != pdPASS)
        {
        g_filtered_drop_cnt++;
        }
    }
}
