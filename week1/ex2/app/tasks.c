/* Week 1 / Ex2 — the three competing tasks.
 *
 * Each task: raise its pin, burn exactly TASK_x_WORK_US of CPU, lower its pin,
 * and loop again WITHOUT blocking. The missing block is the point of the
 * exercise, not a bug to fix in step 2.
 */
#include "FreeRTOS.h"

#include "main.h"
#include "params.h"
#include "tasks.h"
#include "task.h"
/* TODO: one helper that takes a port/pin and a work time, then three thin
 *       wrappers. Do not copy-paste the body three times. */

#define gpio_task_port GPIOA
#define gpio_taska_pin  ((uint16_t)(1U << (TASK_A_PIN)))
#define gpio_taskb_pin  ((uint16_t)(1U << (TASK_B_PIN)))
#define gpio_taskc_pin  ((uint16_t)(1U << (TASK_C_PIN)))

static void alltask(GPIO_TypeDef *port,uint16_t pin,uint32_t time){
    HAL_GPIO_TogglePin(port,pin);
    dwt_busy_wait_us(time);
}
void task_a(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO */
        alltask(gpio_task_port,gpio_taska_pin,TASK_A_WORK_US);
    }
}

void task_b(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO */
        alltask(gpio_task_port,gpio_taskb_pin,TASK_B_WORK_US);
        
    }
}
void task_c(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO */
        alltask(gpio_task_port,gpio_taskc_pin,TASK_C_WORK_US);
    }
}
