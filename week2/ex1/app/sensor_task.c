/* Week 2 / Ex1 — reads MPU6050 over I2C1 every SAMPLE_PERIOD_MS, pushes a
 * raw_sample_t into queue_raw.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

extern I2C_HandleTypeDef hi2c1;         


void sensor_task(void *argument)
{
    (void)argument;
    uint32_t seq = 0;

    /* TODO 1: HAL_I2C init / verify MPU6050 WHO_AM_I. Do this once here, not
     *         inside the loop. */

    const uint16_t mpu_addr = (0x68U << 1);      /* HAL dung dia chi 8 bit */
    uint8_t who  = 0U;
    uint8_t wake = 0U;                           /* ghi 0 vao PWR_MGMT_1 de danh thuc chip */

    for (;;) {
        if ((HAL_I2C_Mem_Read(&hi2c1, mpu_addr, 0x75U, I2C_MEMADD_SIZE_8BIT,
                              &who, 1U, 3U) == HAL_OK)
            && (who == 0x68U)
            && (HAL_I2C_Mem_Write(&hi2c1, mpu_addr, 0x6BU, I2C_MEMADD_SIZE_8BIT,
                                  &wake, 1U, 3U) == HAL_OK)) {
            break;                               /* khoi tao xong */
        }
        vTaskDelay(pdMS_TO_TICKS(100));         /* loi: nghi roi thu lai, co block */
    }

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        raw_sample_t s;
        s.seq = seq++;

        /* TODO 2: HAL_I2C_Mem_Read the accel registers into s.ax/ay/az. */
        uint8_t buf[6];
        uint8_t ok = (HAL_I2C_Mem_Read(&hi2c1, mpu_addr, 0x3BU, I2C_MEMADD_SIZE_8BIT,
                                       buf, 6U, 3U) == HAL_OK) ? 1U : 0U;
        if (ok != 0U) {
            s.ax = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]);   /* big-endian */
            s.ay = (int16_t)(((uint16_t)buf[2] << 8) | buf[3]);
            s.az = (int16_t)(((uint16_t)buf[4] << 8) | buf[5]);
        }

        /* TODO 3: xQueueSend(queue_raw, &s, <timeout>) — pick and justify the
         *         timeout in PREDICTION.md question 3/4. What happens to this
         *         sample if the queue is full and the timeout expires? */
        if (ok != 0U) {
            /* Timeout 0: producer co chu ky cung 4 ms, khong duoc block.
             * Queue day -> mau bi bo, seq nhay coc o phia sau. */
            (void)xQueueSend(queue_raw, &s, 0);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}