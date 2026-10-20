
/* Week 1 / Ex4 — entry point. */

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "ram_report.h"
#include <stdio.h>
/* Configuration */
#define ENABLE_HOG              0

#ifndef RAM_REPORT_STACK_WORDS
#define RAM_REPORT_STACK_WORDS  200U
#endif

#define PRIO_WORKER             (tskIDLE_PRIORITY + 1)
#define PRIO_REPORT             (tskIDLE_PRIORITY + 2)

/* Task functions */
void ram_report_task(void *argument);
static void dyn_task(void *argument);
static void stat_task(void *argument);
static void another_task(void *argument);

/* Task handles */
TaskHandle_t dyn_task_Handle = NULL;
TaskHandle_t ram_report_task_Handle = NULL;
TaskHandle_t stat_task_Handle = NULL;

/* Static allocation for stat_task */
static StaticTask_t stat_task_TCB;
static StackType_t stat_task_Stack[STATIC_TASK_STACK_WORDS];

/* Shared work: both measured tasks perform the same operation. */
static void shared_work(void *argument)
{
    (void)argument;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void dyn_task(void *argument)
{
    (void)argument;

#if (ENABLE_HOG == 1)
    /* Enable only for the intentional stack-overflow experiment. */
    volatile uint8_t ucHog[HOG_DEPTH_BYTES];

    for (uint16_t i = 0; i < sizeof(ucHog); i++) {
    ucHog[i] = 0x36;
}
#endif

    shared_work(argument);
}

static void stat_task(void *argument)
{
    shared_work(argument);
}

/* Retained from file 1. This task continuously consumes CPU time. */
static void another_task(void *argument)
{
    (void)argument;

    for (;;) {
    }
}

void app_main(void)
{
    /* 1. Create dyn_task dynamically. */
    BaseType_t state_1 = xTaskCreate(
        dyn_task,
        "dyn task",
        DYN_TASK_STACK_WORDS,
        NULL,
        PRIO_WORKER,
        &dyn_task_Handle
    );

    /* 2. Create the RAM reporting task dynamically. */
    BaseType_t state_2 = xTaskCreate(
        ram_report_task,
        "ram report task",
        RAM_REPORT_STACK_WORDS,
        NULL,
        PRIO_REPORT,
        &ram_report_task_Handle
    );

    /* 3. Create another_task, as in file 1. */
    BaseType_t state_3 = xTaskCreate(
        another_task,
        "another task",
        128,
        NULL,
        PRIO_WORKER,
        NULL
    );

    /* 4. Create stat_task using statically allocated memory. */
    stat_task_Handle = xTaskCreateStatic(
        stat_task,
        "stat task",
        STATIC_TASK_STACK_WORDS,
        NULL,
        PRIO_WORKER,
        stat_task_Stack,
        &stat_task_TCB
    );

    /* Check every task-creation result. */
    if (state_1 != pdPASS) {
        Uart_print("Create failed: dyn task\r\n");
    }

    if (state_2 != pdPASS) {
        Uart_print("Create failed: ram report task\r\n");
    }

    if (state_3 != pdPASS) {
       Uart_print("Create failed: another task\r\n");
    }

    if (stat_task_Handle == NULL) {
       Uart_print("Create failed: stat task\r\n");
    }

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Reaching here usually means the scheduler could not start. */
    for (;;) {
    }
}