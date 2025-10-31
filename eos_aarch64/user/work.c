#include <core/eos.h>
#if 0

#define STACK_SIZE 4096

static eos_tcb_t t_head, t_follow, t_release;
static int32u_t stack_head[STACK_SIZE], stack_follow[STACK_SIZE], stack_release[STACK_SIZE];
static eos_semaphore_t sem1;

static void dump_queue_details(const char *tag)
{
    PRINT("-- %s -- \n", tag);
    PRINT("     sem1.wait_queue = %p | head.queue_node = %p | follow.queue_node = %p\n",
          (void *)sem1.wait_queue, (void *)&t_head.queue_node, (void *)&t_follow.queue_node);

    // 큐 상태 분석
    if (sem1.wait_queue == NULL)
    {
        PRINT("     STATUS: Queue is EMPTY \n");
    }
    else if (sem1.wait_queue == &t_head.queue_node)
    {
        PRINT("     STATUS: Queue head = HEAD task (POTENTIAL BUG if head timed out!) ❌\n");
    }
    else if (sem1.wait_queue == &t_follow.queue_node)
    {
        PRINT("     STATUS: Queue head = FOLLOW task ✅\n");
    }
    else
    {
        PRINT("     STATUS: Queue head = UNKNOWN address ⚠️\n");
    }

    PRINT("     head.status = %d, follow.status = %d\n", t_head.status, t_follow.status);
}

static void head_task(void *arg)
{
    PRINT("[HEAD_TASK] Acquiring semaphore (timeout=2 ticks)...\n");

    int result = eos_acquire_semaphore(&sem1, 2);
    
    if (result)
    {
        PRINT("[HEAD_TASK] ❌ UNEXPECTED: Got semaphore (should have timed out!)\n");
        eos_release_semaphore(&sem1);
    }
    else
    {
        PRINT("[HEAD_TASK] ✅ Timed out as expected\n");
    }
    
    dump_queue_details("After HEAD timeout");
    PRINT("[HEAD_TASK] Task endings\n");
    while (1)
        eos_sleep(100);
}

static void follower_task(void *arg)
{
    eos_sleep(1); // Wait for head to acquire first
    PRINT("[FOLLOW] Acquiring semaphore (infinite wait)...\n");
    int result = eos_acquire_semaphore(&sem1, 0);
    if (result)
    {
        PRINT("[FOLLOW] ✅ SUCCESS: Finally got semaphore!\n");
        dump_queue_details("After FOLLOW success");
        eos_release_semaphore(&sem1);
    }
    else
    {
        PRINT("[FOLLOW] ❌ FAILED: This should never happen with timeout=0\n");
    }

    dump_queue_details("After FOLLOW success");
    PRINT("[FOLLOW] Task ending\n");
    while (1)
        eos_sleep(100);
}

static void releaser_task(void *arg)
{
    eos_sleep(3); // Wait for head timeout

    PRINT("[RELEASER] About to release semaphore\n");
    dump_queue_details("Before RELEASE");

    eos_release_semaphore(&sem1);

    dump_queue_details("After RELEASE");

    // 잠깐 기다려서 follower 반응 확인
    eos_sleep(1);
    PRINT("[RELEASER] Checking if follower woke up...\n");
    dump_queue_details("Final check");

    PRINT("[RELEASER] Task ending\n");
    while (1)
        eos_sleep(100);
}

void eos_user_main()
{
    eos_init_semaphore(&sem1, 0, 0); // count=0, FIFO

    eos_create_task(&t_head, (addr_t)stack_head, STACK_SIZE * sizeof(int32u_t), head_task, NULL, 10);
    eos_create_task(&t_follow, (addr_t)stack_follow, STACK_SIZE * sizeof(int32u_t), follower_task, NULL, 20);
    eos_create_task(&t_release, (addr_t)stack_release, STACK_SIZE * sizeof(int32u_t), releaser_task, NULL, 30);

    PRINT("🔍 TIMEOUT BUG DETECTION TEST 🔍\n");
    dump_queue_details("INITIAL STATE");
    PRINT("========================\n");
    PRINT("📋 Test Plan:\n");
    PRINT("1. HEAD acquires (timeout=2) → should become queue head\n");
    PRINT("2. FOLLOW acquires (infinite) → should become queue tail\n");
    PRINT("3. HEAD times out → queue head should change to FOLLOW\n");
    PRINT("4. RELEASER releases → should wake FOLLOW\n");
    PRINT("🚨 If BUG exists: Queue head stays at HEAD after timeout!\n");
    PRINT("✅ If FIXED: Queue head changes to FOLLOW after timeout!\n");
    PRINT("========================\n");
}
#endif