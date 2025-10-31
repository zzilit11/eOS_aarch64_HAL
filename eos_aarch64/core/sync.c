/********************************************************
 * Filename: core/sync.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/31/24
 *
 * Description: Routines for semaphores and condition variables
 ********************************************************/

#include <core/eos.h>

void eos_init_semaphore(eos_semaphore_t *sem, int8s_t initial_count, int8u_t queue_type)
{
    if (sem == NULL) {
        PRINT("semaphore is NULL\n");
        return;
    }
    // To be filled by students: Project 4
    sem->count = initial_count;
    sem->wait_queue = NULL;
    sem->queue_type = queue_type;
}


int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout)
{
    // To be filled by students: Project 4
    if (sem == NULL) {
        PRINT("semaphore is NULL\n");
        return 0;
    }

    /* Check if the scheduler is locked */
    if (eos_get_scheduler_lock()) {
        PRINT("Scheduler locked. eos_sem_acquire() failed.\n");
        return 0;
    }

    /* Obtain the timeout point in the absolute time scale */
    int32u_t abs_timeout = eos_get_system_timer()->tick + (int32u_t) timeout;

    int32u_t flag = hal_disable_interrupt();
    if (sem->count <= 0) {
        /* The semaphore is already locked */
        if (timeout < 0) {
            hal_restore_interrupt(flag);
            return 0;
        }
        eos_set_alarm(eos_get_system_timer(),
                      &eos_get_current_task()->alarm,
                      (int32u_t) timeout, _os_wakeup_from_alarm_queue,
                      eos_get_current_task());
        do {
            hal_restore_interrupt(flag);
            _os_wait_in_queue(&sem->wait_queue, sem->queue_type);

            if (timeout != 0 && abs_timeout <= eos_get_system_timer()->tick) {
                /* This task is waken up by alarm */
                return 0;
            }

            flag = hal_disable_interrupt();
         } while (sem->count <= 0);

         /* Remove the alarm (This task is waken up by another task) */
         eos_set_alarm(eos_get_system_timer(),
                       &eos_get_current_task()->alarm, 0, NULL, NULL);
    }

    sem->count--;
    hal_restore_interrupt(flag);

    return 1;

}


void eos_release_semaphore(eos_semaphore_t *sem)
{
    // To be filled by students: Project 4
    if (sem == NULL) {
        PRINT("semaphore is NULL\n");
        return;
    }
    
    int32u_t flag = hal_disable_interrupt();
    sem->count++;
    hal_restore_interrupt(flag);
    if(sem->wait_queue) {
            /* Select a task from the wait queue and make it ready */
            _os_wakeup_from_queue(&sem->wait_queue);
    }

}


/**
 * Condition variables are not covery in the OS course
 */

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type)
{
    if (cond == NULL) {
        PRINT("cond is NULL\n");
        return;
    }

    cond->wait_queue = NULL;
    cond->queue_type = queue_type;
}


void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex)
{
    if (cond == NULL || mutex == NULL) {
        PRINT("invalid cond(%p) or mutex(%p)\n", (void*)cond, (void*)mutex);
        return;
    }

    /* Releases acquired semaphore */
    eos_release_semaphore(mutex);
    /* Waits on condition's wait_queue */
    _os_wait_in_queue(&cond->wait_queue, cond->queue_type);
    /* Acquires semaphore before returns */
    eos_acquire_semaphore(mutex, 0);
}


void eos_notify_condition(eos_condition_t *cond)
{
    if (cond == NULL) {
        PRINT("cond is NULL\n");
        return;
    }

    /* Selects a task in wait_queue and wake it up */
    _os_wakeup_from_queue(&cond->wait_queue);
}


int8u_t eos_lock_scheduler() {
    return _os_lock_scheduler();
}


void eos_restore_scheduler(int8u_t scheduler_state) {
    _os_restore_scheduler(scheduler_state);
}


int8u_t eos_get_scheduler_lock() {
    return _os_scheduler_lock;
}

