#include <core/eos.h>

#define STACK_SIZE 8096

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static eos_tcb_t tcb3;
static int32u_t stack1[STACK_SIZE];
static int32u_t stack2[STACK_SIZE];
static int32u_t stack3[STACK_SIZE];
static int32u_t queue1[10];
static int32u_t queue2[10];
eos_mqueue_t mq1;
eos_mqueue_t mq2;

static void sender_task(void *arg)
{
    int32u_t *data = "xy";

    while (1)
    {
        PRINT("send message to mq1\n");
        eos_send_message(&mq1, data, 0);
        PRINT("send message to mq2\n");
        eos_send_message(&mq2, data, 0);
        eos_sleep(0);
    }
}

static void receiver_task1(void *arg)
{
    int32u_t data[2];

    while (1)
    {
        PRINT("receive message from mq1\n");
        eos_receive_message(&mq1, data, 0);
        PRINT("received message: %s\n", data);
        eos_sleep(0);
    }
}

static void receiver_task2(void *arg)
{
    int32u_t data[2];

    while (1)
    {
        PRINT("receive message from mq2\n");
        eos_receive_message(&mq2, data, 0);
        PRINT("received message: %s\n", data);
        eos_sleep(0);
    }
}

void eos_user_main()
{
    eos_create_task(&tcb1, (addr_t)stack1, 8096, sender_task, NULL, 50);
    eos_create_task(&tcb2, (addr_t)stack2, 8096, receiver_task1, NULL, 20);
    eos_create_task(&tcb3, (addr_t)stack3, 8096, receiver_task2, NULL, 10);

    eos_set_period(&tcb1, 2);
    eos_set_period(&tcb2, 4);
    eos_set_period(&tcb3, 6);

    eos_init_mqueue(&mq1, queue1, 5, 2, PRIORITY);
    eos_init_mqueue(&mq2, queue2, 5, 2, PRIORITY);
}