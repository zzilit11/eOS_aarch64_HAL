#include <core/eos.h>

#define TASK_NUMBER             10
#define CPU_TASK_NUMBER         4
#define IO_TASK_NUMBER          3
#define SEND_TASK_NUMBER        3
#define RECEIVE_TASK_NUMBER     3
#define MESSAGE_QUEUE_SIZE      5
#define MESSAGE_LENGTH          2


#define TEST_STACK_SIZE         4096
#define TEST_MAX_LOOP_COUNT     0x01FFFFFF

static void test_task_math(void* arg);
static void test_task_send(void* arg);
static void test_task_receive(void* arg);
static void test_send1(void *arg);
static void test_receive1(void *arg);

int test_create_task();
int test_math(int TaskId);
int test_send(int TaskId);
int test_receive(int TaskId);
