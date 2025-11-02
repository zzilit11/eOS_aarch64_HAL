/********************************************************
 * Filename: core/task.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 04/06/24
 * Modified by: Geonha Park, Jongwon Kim, Jigyo Kim on 09/14/25
 *
 * Description: task creation and scheduling
 ********************************************************/

#include <core/eos.h>

#define READY		    1
#define RUNNING		    2
#define WAITING		    3
#define SUSPENDED       4

#define MIN_STACK_SIZE 1024
/**
 * Runqueue of ready tasks
 */
static _os_spinlock_t _os_ready_queue_lock = SPINLOCK_UNLOCKED;
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];
// 우선순위 0~63까지 총 64개의 우선순위를 지원하므로, 배열 크기는 LOWEST_PRIORITY + 1 (64)로 설정
// 각 배열 원소는 해당 우선순위에 있는 태스크들 연결 리스트의 헤드 노드를 가리킴: 주소값을 저장하는 포인터 변수

/**
 * Pointer to TCB of the running task
 */
static eos_tcb_t *_os_current_task;
// 현재 실행 중인 태스크의 TCB를 가리키는 포인터 변수


int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority)
{
    /* Validate parameters */
    if (task == NULL || entry == NULL) {
        PRINT("invalid task(%p) or entry(%p)\n", (void*)task, (void*)entry);
        return (int32u_t)-1;
    }
    if (priority > LOWEST_PRIORITY) {
        PRINT("invalid priority=%u\n", priority);
        return (int32u_t)-1;
    }
    if (sblock_start == 0 || sblock_size < MIN_STACK_SIZE) {
        PRINT("invalid stack start(%p) or size(%zu)\n", (void*)sblock_start, sblock_size);
        return (int32u_t)-1;
    }

    // To be filled by students: Projects 2 and 3

    PRINT("task: %p, priority: %u\n", (void*)task, priority);

    /* Initializes priority */
    task->priority = priority;

    /* Initializes period */
    task->period = 0;
    task->wakeup_time = 0;

    /* Initializes list-related fields */
    task->queue_node.pnode = task;
    task->queue_node.order_val = task->priority;

    /* Creates a context and store the context in the tcb */
    task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);

    /* Inserts this tcb into the ready queue */
    int32u_t flag = _os_lock_sync(&_os_ready_queue_lock);
    _os_add_node_tail(&_os_ready_queue[task->priority], &(task->queue_node));
    _os_set_ready(task->priority);
    _os_unlock_sync(flag, &_os_ready_queue_lock);
    task->status = READY;

    eos_schedule();

    return 0;
}


int32u_t eos_destroy_task(eos_tcb_t *task)
{
    // To be filled by students: not covered
}


void eos_schedule()
{
    /* Checks if the scheduler is locked */
    int32u_t flag = hal_disable_interrupt();
    if (_os_scheduler_lock == LOCKED) {
        /* Preemption is disabled: Do nothing */
        hal_restore_interrupt(flag);
        return;
    }
    hal_restore_interrupt(flag);

    if (_os_current_task) {
        if (_os_current_task->status == RUNNING) {
            /* Inserts the running task into the ready queue */
            _os_add_node_tail(&_os_ready_queue[_os_current_task->priority],
                            &(_os_current_task->queue_node));
            _os_set_ready(_os_current_task->priority);
            _os_current_task->status = READY;
        }

    //    /* Saves the current context */
    //    addr_t sp = _os_save_context();
    //    if (!sp) {
    //        return;  // Return to the preemption point after restoring context
    //    }
//
    //    /* Saves the stack pointer in the tcb */
    //    _os_current_task->sp = sp;
    } else {
        /* Reaches here when eOS call eos_schedule(): Only runs the next task */
    }

    /* Selects the next task to run */
    int32u_t highest_priority = _os_get_highest_priority();
    _os_node_t *node = _os_ready_queue[highest_priority];
    eos_tcb_t *next_task = (eos_tcb_t *) node->pnode;

    /* Removes the selected task from the ready queue */
    _os_remove_node(&_os_ready_queue[next_task->priority], node);
    if (!_os_ready_queue[next_task->priority])
        _os_unset_ready(next_task->priority);

    /* Restores the context of the next task */
    next_task->status = RUNNING;
    _os_current_task = next_task;
    _os_restore_context(next_task->sp);

    /* Never reaches here */
}


eos_tcb_t *eos_get_current_task()
{
	return _os_current_task;
}


void eos_change_priority(eos_tcb_t *task, int32u_t priority)
{
    if (task == NULL || priority > LOWEST_PRIORITY) {
        PRINT("invalid task(%p) or priority=%u\n", (void*)task, priority);
        return;
    }
	/* if task is READY, update bitmap and ready_queue */
	if (task->status == READY) {
		_os_remove_node(&_os_ready_queue[task->priority], &(task->queue_node));
		if (!_os_ready_queue[task->priority])
			_os_unset_ready(task->priority);

		_os_add_node_tail(&_os_ready_queue[priority], &(task->queue_node));
		_os_set_ready(priority);
	}

	/* change the tcb */
	task->priority = priority;
	task->queue_node.order_val = task->priority;
	
	/* schedule with new priority set */
	eos_schedule();
}


int32u_t eos_get_priority(eos_tcb_t *task)
{
	return task->priority;
}


void eos_set_period(eos_tcb_t *task, int32u_t period)
{
    // To be filled by students: Project 3
    task->period = period;
    task->wakeup_time = eos_get_system_timer()->tick;
}


int32u_t eos_get_period(eos_tcb_t *task)
{
	return task->period;
}


int32u_t eos_suspend_task(eos_tcb_t *task)
{
    if (task == NULL) {
        PRINT("task is NULL\n");
        return (int32u_t)-1;
    }

	if (task->status == READY) {
		_os_remove_node(&_os_ready_queue[task->priority], &(task->queue_node));
		if (!_os_ready_queue[task->priority])
			_os_unset_ready(task->priority);
		task->status = SUSPENDED;
	}
	return 0;
}


int32u_t eos_resume_task(eos_tcb_t *task)
{
    if (task == NULL) {
        PRINT("task is NULL\n");
        return (int32u_t)-1;
    }
	if (task->status == SUSPENDED) {
		_os_add_node_tail(&_os_ready_queue[task->priority], &(task->queue_node));
		_os_set_ready(task->priority);
		task->status = READY;
		eos_schedule();
	}
	return 0;
}


void eos_sleep(int32u_t tick)
{
    // To be filled by students: Project 3
    /* Check if scheduler is locked */
    if (eos_get_scheduler_lock() == LOCKED) {
        PRINT("Can't sleep since scheduler is locked\n");
        return;
    }

    int32u_t timeout = tick;
        
    if (tick == 0) { // tick을 0으로 지정한 경우, 0tick 동안 sleep하는 것이 아니라, 다음 주기까지 sleep하는 것으로 사용함.
        if(_os_current_task->period != 0) {
            /* The current task is periodic */
            _os_current_task->wakeup_time += _os_current_task->period;
            if (_os_current_task->wakeup_time <= eos_get_system_timer()->tick) {
                PRINT("There exist queued jobs, so execute them\n");
                return;
            }
            timeout = _os_current_task->wakeup_time - eos_get_system_timer()->tick;
        }
    }

    eos_set_alarm(eos_get_system_timer(), &_os_current_task->alarm, timeout, _os_wakeup_from_alarm_queue, _os_current_task);

    /* Goes to the WAITING state */
    _os_current_task->status = WAITING;

    /* Selects a task from the ready list and runs it */
    eos_schedule();
}


void _os_init_task() // 확인 완료 (25/09/07-이종원)
{
    PRINT("Initializing task module\n");

    /* Initializes current_task */
    _os_current_task = NULL; 

    /* Initializes multi-level ready_queue */
    for (int32u_t i = 0; i <= LOWEST_PRIORITY; i++) { //기존, i < LOWEST_PRIORITY로 되어있던 코드 수정 (25/09/07-이종원)
        _os_ready_queue[i] = NULL;
    }
}


void _os_wait_in_queue(_os_node_t **wait_queue, int8u_t queue_type)
// 역할: 현재 실행 중인 태스크를 지정된 대기 큐에 삽입하고, 
// 해당 태스크를 WAITING 상태로 변경한 후, 
// 스케줄러를 호출하여 다른 태스크를 실행
// wait_queue: 대기 큐의 헤드 노드를 가리키는 포인터 변수의 주소 -> *wait_queue는 헤드 노드의 주소, **wait_queue는 헤드 노드 자체
// 어떤 wait_queue에 삽입될지가, 이때의 input으로 받는 값에 의해 결정 (ex) sem->wait_queue, cond->wait_queue 등)
// queue_type: 대기 큐에서 테스크를 선택하는 기준을 지정 -> 0: FIFO, 1: 우선순위 기반

{
    // To be filled by students: Project 4
    if (!queue_type) { // FIFO 방식
        _os_add_node_tail(wait_queue, &_os_current_task->queue_node);
        // FIFO면 현재 태스크를 대기 큐의 맨 뒤에 삽입
    } else {
        _os_add_node_ordered(wait_queue, &_os_current_task->queue_node);
        // PRIORITY면 현재 태스크를 우선순위에 따라 대기 큐에 삽입
    }

    _os_current_task->wait_queue_owner = wait_queue;
    _os_current_task->status = WAITING;

    
    eos_schedule();

}


void _os_wakeup_from_queue(_os_node_t **wait_queue)
{
    // To be filled by students: Project 4
    if(*wait_queue == NULL) return ;

    /* Get the first task */
    eos_tcb_t *task = (eos_tcb_t*) (*wait_queue)->pnode;

    /* Remove task from wait_queue */
    _os_remove_node(wait_queue, &task->queue_node);

    _os_add_node_tail(&_os_ready_queue[task->priority], &task->queue_node);
    _os_set_ready(task->priority);
    task->status = READY;

    eos_schedule();
  }


void _os_wakeup_all_from_queue(_os_node_t **wait_queue)
{
    // To be filled by students: not covered
}


void _os_wakeup_from_alarm_queue(void *arg)
{
    // To be filled by students: Project 3
    eos_tcb_t *task = (eos_tcb_t *) arg;

    _os_node_t **wait_queue_owner = task->wait_queue_owner;
    if (wait_queue_owner != NULL) {
        _os_remove_node(wait_queue_owner, &task->queue_node); // Remove current task from its wait queue that it was waiting on
        task->wait_queue_owner = NULL; // Clear the task's wait_queue_owner after removing from wait queue
    }

    _os_add_node_tail(&_os_ready_queue[task->priority], &(task->queue_node));
    _os_set_ready(task->priority);
    task->status = READY;

}
