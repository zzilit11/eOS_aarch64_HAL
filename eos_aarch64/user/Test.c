#include <stdio.h>
#include <math.h>
#include "Test.h"
#if 1
static int gCpuTaskId[CPU_TASK_NUMBER];
static eos_tcb_t gCpuTaskTcb[CPU_TASK_NUMBER];
static int8u_t gCpuTaskStack[CPU_TASK_NUMBER][TEST_STACK_SIZE] __attribute__((aligned(sizeof(uintptr_t))));

static int gSendTaskId[IO_TASK_NUMBER];
static eos_tcb_t gSendTaskTcb[IO_TASK_NUMBER];
static int8u_t gSendTaskStack[IO_TASK_NUMBER][TEST_STACK_SIZE] __attribute__((aligned(sizeof(uintptr_t))));

static int gReceiveTaskId[IO_TASK_NUMBER];
static eos_tcb_t gReceiveTaskTcb[IO_TASK_NUMBER];
static int8u_t gReceiveTaskStack[IO_TASK_NUMBER][TEST_STACK_SIZE] __attribute__((aligned(sizeof(uintptr_t))));

static eos_mqueue_t gIoTaskMsg[IO_TASK_NUMBER];
static int8u_t gIoTaskQueue[IO_TASK_NUMBER][MESSAGE_QUEUE_SIZE * MESSAGE_LENGTH] __attribute__((aligned(sizeof(uintptr_t))));

void eos_user_main()
{
    test_create_task();
}

static void test_task_math (void *arg)
{
    int* pTaskId = (int*)arg;
    test_math(*pTaskId);
}

static void test_task_send (void *arg)
{
    int* pTaskId = (int*)arg;
    test_send(*pTaskId);
}

static void test_task_receive (void *arg)
{
    int* pTaskId = (int*)arg;
    test_receive(*pTaskId);
}

int test_create_task()
{
    int priority = 50;
    int period = 1;
    for(int index = 0; index < CPU_TASK_NUMBER; index ++)
    {
        gCpuTaskId[index] = index;
        eos_create_task(&gCpuTaskTcb[index], gCpuTaskStack[index], sizeof(gCpuTaskStack[index]), test_task_math, &gCpuTaskId[index], priority);
    }

    priority = 10;
    for(int index = 0; index < IO_TASK_NUMBER; index ++)
    {
        priority += 5;
        period ++;
        gSendTaskId[index] = index;
        eos_create_task(&gSendTaskTcb[index], gSendTaskStack[index], sizeof(gSendTaskStack[index]), test_send1, &gSendTaskId[index], priority);
        eos_set_period(&gSendTaskTcb[index], period);
        priority += 5;
        gReceiveTaskId[index] = index;
        eos_create_task(&gReceiveTaskTcb[index], gReceiveTaskStack[index], sizeof(gReceiveTaskStack[index]), test_receive1, &gReceiveTaskId[index], priority);
        eos_set_period(&gReceiveTaskTcb[index], period);
        eos_init_mqueue(&gIoTaskMsg[index], gIoTaskQueue[index], MESSAGE_QUEUE_SIZE, MESSAGE_LENGTH, PRIORITY);
    }
    return 0;
}

int test_math(int TaskId)
{
    int count = 0;
    double result = 0.0f;
    double value = 0.0f;
    double fnumber = 0.0f;
    int inumber1 = 0;
    int inumber2 = 0;

    while(1)
    {
        result += (fnumber / (value + 1)) + (inumber1 % (inumber2 + 1));
        value += 1;
        fnumber = value * value;
        inumber1 += 1;
        inumber2 = inumber1 * inumber1;
        if(value > TEST_MAX_LOOP_COUNT)
        {
            unsigned long integer_part = (unsigned long)result;
            double fractional = result - (double)integer_part;
            unsigned long fractional_scaled;

            if (fractional < 0) {
                fractional = -fractional;
            }

            fractional_scaled = (unsigned long)(fractional * 1000000.0);
            if (fractional_scaled >= 1000000UL) {
                fractional_scaled = 0;
                integer_part += 1;
            }

            count ++;
            PRINT("CPU Task ID = %d, loop count = %d, Result = %lu.%06lu\n", TaskId, count, integer_part, fractional_scaled);
            value = 0.0f;
        }
    }
    return 0;
}

int test_send(int TaskId)
{
    int32u_t data = 1;
    int32u_t count = 1;
    while (1)
    {
        data = count;
        PRINT("Send Task ID = %d, count = %d, message = %u\n", TaskId, count, data);
        eos_send_message(&gIoTaskMsg[TaskId], &data, 0);
        count ++;
        eos_sleep(0);
    }
    return 0;
}

int test_receive(int TaskId)
{
    int32u_t data = 1;
    int32u_t count = 1;
    while (1)
    {
        eos_receive_message(&gIoTaskMsg[TaskId], &data, 0);
        PRINT("Receive Task ID = %d, count = %d, message = %u\n", TaskId, count, data);
        count ++;
        eos_sleep(0);
    }
    return 0;
}

static void test_send1(void *arg)
{
    int* pTaskId = (int*)arg;
    char data[MESSAGE_LENGTH] = {'x', 'y'};
    int32u_t count = 0;
    while (1)
    {
        count ++;
        PRINT("Send Task ID = %d, count = %d, message = %.*s\n", *pTaskId, count, MESSAGE_LENGTH, data);
        eos_send_message(&gIoTaskMsg[*pTaskId], data, 0);
        eos_sleep(0);
    }
}

static void test_receive1(void *arg)
{
    int* pTaskId = (int*)arg;
    char data[MESSAGE_LENGTH + 1];
    int32u_t count = 0;
    while (1)
    {
        count ++;
        eos_receive_message(&gIoTaskMsg[*pTaskId], data, 0);
        data[MESSAGE_LENGTH] = '\0';
        PRINT("Receive Task ID = %d, count = %d, message = %s\n", *pTaskId, count, data);
        eos_sleep(0);
    }
}
#endif
