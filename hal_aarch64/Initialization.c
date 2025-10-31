/*
    Replace core/main.c
    No more need for main() function in QEMU environment
*/
#include "type.h"
#include "uart.h"
#include "interrupt.h"
#include "timer.h"
#include "context.h"
#include "core/eos_internal.h"


__attribute__((aligned(16), section(".text")))
void _os_initialization(void) {
    /* disable interrupt */
    eos_disable_interrupt();

    /* lock the scheduler */
    _os_scheduler_lock = SCHEDULER_LOCKED;

    _os_init_hal();
    _os_init_icb_table();
}