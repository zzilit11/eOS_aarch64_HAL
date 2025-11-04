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
static int8u_t idle_stack[8096];	// stack for the idle task


/*
 * This function is called by HAL after initializing H/W
 */
void _os_init() //초기화 함수로, hal의 entry.S에서 호출됨
{
    // Interrupts and preemption must be disabled during initialziation
    hal_disable_interrupt(); // hal/interrupt_asm.s에 구현되어 있음. // 확인 완료(25/09/07-이종원)
    _os_scheduler_lock = LOCKED; //eos_internal.h에 int8u_t로 선언되어 있음 + scheduler.c에 정의되어 있음 // 확인 완료(25/09/07-이종원)

    // Initializes subsystems
    _gic_init();
    _os_init_hal(); // timer interrupt 만 활성화함
    _os_init_icb_table(); //core/interrupt.c에 구현되어 있음 - Team A 관할 // 확인 완료(25/09/07-이종원)
    _os_init_scheduler(); // core/scheduler.c에 구현되어 있음 - Team A 관할 //확인 완료 (25/09/07-이종원)
    _os_init_task(); // core/task.c에 구현되어 있음 - Team A 관할 //확인 완료 (25/09/07-이종원)
    _os_init_timer(); // core/timer.c에 구현되어 있음 - Team A 관할 //진행중 (25/09/07-이종원)

    // Creates an idle task
    PRINT("Creating an idle task\n");
    eos_create_task(&idle_task, idle_stack, sizeof(idle_stack), _os_idle_task, NULL, LOWEST_PRIORITY); // core/task.c에 구현

    void eos_user_main();
    eos_user_main();

    // Starts multitasking by enabling preemption and interrupts
    PRINT("Starts multitasking\n");
    _os_scheduler_lock = UNLOCKED; // UNLOCKED = 0
    hal_enable_interrupt(); 

    // Permanently gives control to the tasks in the ready queue
    // The idle task runs when the ready queue is effectively empty
    eos_schedule();

    // Control never reaches here
    while (1) { PRINT("in the loop\n"); }
}


static void _os_idle_task(void *arg)
{
    while (1) {
        //PRINT("Idle task running...\n"); 
    } 
}
