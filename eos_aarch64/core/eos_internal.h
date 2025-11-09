/********************************************************
 * Filename: core/eos_internal.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 04/07/24
 *
 * Description: Internal header for eOS
 ********************************************************/

#ifndef EOS_INTERNAL_H
#define EOS_INTERNAL_H

#include <stdio.h>
#include <stdarg.h>
#include <hal/aarch64/type.h>
#include <hal/aarch64/context.h>
#include <hal/aarch64/timer.h>


/********************************************************
 * Initialization module
 ********************************************************/

void _os_init();		    // Initialize OS
void _os_init_hal();		// Initialize HAL
void _os_init_icb_table();	// Initialize ICB table structure
void _os_init_scheduler();	// Initialize bitmap scheduler module
void _os_init_task();		// Initialize task management module
// void _os_init_timer();		// Initialize timer management module


/********************************************************
 * Lock module
 ********************************************************/
#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCKED   1
typedef volatile int _os_spinlock_t;
void _os_spin_lock(_os_spinlock_t *lock);
void _os_spin_unlock(_os_spinlock_t *lock);

/********************************************************
 * Common utility module
 ********************************************************/

/* Common structure for list element */
typedef struct _os_node {
    struct _os_node *prev;
    struct _os_node *next;
    void *pnode; //연결 리스트의 실제 데이터 주소
    int32u_t order_val;
} _os_node_t;

/* Adds the specified node at the end (tail) of the list */
void _os_add_node_tail(_os_node_t **head, _os_node_t *new_node);

/* Adds the specified node by its priority */
void _os_add_node_ordered(_os_node_t **head, _os_node_t *new_node);

/* Removes a node from the list */
int32u_t _os_remove_node(_os_node_t **head, _os_node_t *node);

/* Formatted output conversion */
int32s_t vsprintf(char *buf, const char *fmt, va_list args);

void _os_serial_puts(const char *s);


/********************************************************
 * Interrupt management module
 ********************************************************/

/* Maximum number of IRQs */
#define IRQ_MAX 32

/* The common interrupt handler:
 * 	Invoked by HAL whenever an interrupt occurrs.
 */
void _os_common_interrupt_handler(addr_t saved_context_ptr, int32u_t irq_num);

void _os_sync_exception_handler(addr_t saved_context_ptr, int32u_t svc_num);


/********************************************************
 * Timer management module
 ********************************************************/

void _os_init_timer();


/********************************************************
 * Task management module
 ********************************************************/

void _os_wait_in_queue(_os_node_t **wait_queue, int8u_t queue_type);
void _os_wakeup_from_queue(_os_node_t **wait_queue);
void _os_wakeup_all_from_queue(_os_node_t **wait_queue);
void _os_wakeup_from_alarm_queue(void *arg);


/********************************************************
 * Scheduler module
 ********************************************************/

#define LOWEST_PRIORITY		63
#define MEDIUM_PRIORITY		32
#define READY_TABLE_SIZE	(LOWEST_PRIORITY / 8 + 1)
#define LOCKED			1
#define UNLOCKED		0

/* Scheduler lock */
extern int8u_t _os_scheduler_lock;

int8u_t _os_lock_scheduler(void);

void _os_restore_scheduler(int8u_t);

/* Multi-core lock*/
int32u_t _os_lock_sync(_os_spinlock_t *lock);
void _os_unlock_sync(int32u_t flag, _os_spinlock_t *lock);

/* Gets the highest-prioity task from the ready list */
int32u_t _os_get_highest_priority();

/* Sets priority bit in the ready list to 0 */
void _os_unset_ready(int8u_t priority);

/* Sets priority bit in the ready list to 1 */
void _os_set_ready(int8u_t priority);


/********************************************************
 * Hardware abstraction module
 ********************************************************/

/**
 * Creates an initial context on the stack
 * Its stack_pointer is the highest memory address of the stack area
 * "stack_size" is the size of the stack area
 * This function returns task context that it created
 * 
 * When this context is resumed, "entry" is called with the argument "arg"
 */
addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *arg), void *arg);

/**
 * Saves the context of the current task onto stack
 * This method returns in two different ways:
 *     First, it returns after saving the context onto the stack;
 *         it returns the address of the saved context
 *     Second, it returns via the saved task
 *         it returns 0 (NULL)
 */
// addr_t _os_save_context();

/**
 * Restores CPU registers of a given context
 */
// void _os_restore_context(addr_t sp);

#endif /* EOS_INTERNAL_H */
