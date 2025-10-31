/********************************************************
 * Filename: core/init.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 04/07/24
 * 2nd Modified by: Team A on 2025
 * 
 * Description: Perform OS initialization
 ********************************************************/

#include <core/eos.h>

static void _os_idle_task(void *arg);	// idle task
static eos_tcb_t idle_task;		// tcb for the idle task
static int8u_t idle_stack[8096] __attribute__((aligned(sizeof(uintptr_t))));	// stack for the idle task


/*
 * This function is called by HAL after initializing H/W
 */
void _os_init() //초기화 함수로, hal의 entry.S에서 호출됨
{
    // Interrupts and preemption must be disabled during initialziation
    hal_disable_interrupt(); // hal/interrupt_asm.s에 구현되어 있음. // 확인 완료(25/09/07-이종원)
    _os_scheduler_lock = LOCKED; //eos_internal.h에 int8u_t로 선언되어 있음 + scheduler.c에 정의되어 있음 // 확인 완료(25/09/07-이종원)

    // Initializes subsystems
    _os_init_hal(); // hal/init_hal.c에 구현되어 있음 - Team B 관할
    _os_init_icb_table(); //core/interrupt.c에 구현되어 있음 - Team A 관할 // 확인 완료(25/09/07-이종원)
    _os_init_scheduler(); // core/scheduler.c에 구현되어 있음 - Team A 관할 //확인 완료 (25/09/07-이종원)
    _os_init_task(); // core/task.c에 구현되어 있음 - Team A 관할 //확인 완료 (25/09/07-이종원)
    _os_init_timer(); // core/timer.c에 구현되어 있음 - Team A 관할 //진행중 (25/09/07-이종원)

    // Creates an idle task
    PRINT("Creating an idle task\n");
    eos_create_task(&idle_task, idle_stack, sizeof(idle_stack), _os_idle_task, NULL, LOWEST_PRIORITY); // core/task.c에 구현

    // After finishing initialization, calls eos_user_main()
    // user코드에 정의되어 있고, init.c에 선언 및 호출
    // header file에 선언하지 않고, 여기서 선언: user가 직접 호출하는 함수가 아니기 때문
    // user는 해당 이름을 가진 함수에 자신의 코드를 작성하는 형태로 사용
    void eos_user_main();
    eos_user_main();

    // Starts multitasking by enabling preemption and interrupts
    PRINT("Starts multitasking\n");
    _os_scheduler_lock = UNLOCKED; // 검증
    hal_enable_interrupt(); // 검증

    // Permanently gives control to the tasks in the ready queue
    // The idle task runs when the ready queue is effectively empty
    eos_schedule();

    // Control never reaches here
    while (1) { PRINT("in the loop\n"); }
}


static void _os_idle_task(void *arg)
{
    while (1) { /*PRINT("Idle task running...\n");*/ } 
}
