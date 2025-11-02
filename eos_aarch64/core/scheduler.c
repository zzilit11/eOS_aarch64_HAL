/********************************************************
 * Filename: core/scheduler.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/31/24
 *
 * Description: Scheduler routines
 ********************************************************/

#include <core/eos.h>

/* Ready list */
int8u_t _os_ready_group; 
int8u_t _os_ready_table[READY_TABLE_SIZE];

/* Scheduler lock */
int8u_t _os_scheduler_lock;

/* Mapping table */
/* Bit mask table: _os_map_table[index] = (1 << index) */
int8u_t const _os_map_table[] =
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

/* 
 * Lookup table for finding the lowest set bit position in an 8-bit value.
 * _os_unmap_table[value] returns the position of the least significant 1-bit.
 * For example: _os_unmap_table[1] = 0, _os_unmap_table[2] = 1, _os_unmap_table[4] = 2
 * Used in priority bitmap scheduling to find the highest priority task efficiently.
 */
int8u_t const _os_unmap_table[] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};


int8u_t _os_lock_scheduler()
{
    int32u_t flag = hal_disable_interrupt();
    int8u_t temp = _os_scheduler_lock;
    _os_scheduler_lock = LOCKED;
    hal_restore_interrupt(flag);

    return temp;
}


void _os_restore_scheduler(int8u_t scheduler_state)
{
    int32u_t flag = hal_disable_interrupt();
    _os_scheduler_lock = scheduler_state;
    hal_restore_interrupt(flag);
    eos_schedule();
}

int32u_t _os_lock_sync(_os_spinlock_t *lock)
{
    int32u_t flag = hal_disable_interrupt();
    _os_spin_lock(lock);
    return flag;
}

void _os_unlock_sync(int32u_t flag, _os_spinlock_t *lock)
{
    _os_spin_unlock(lock);
    hal_restore_interrupt(flag);
}

void _os_init_scheduler()
{
    PRINT("Initializing scheduler module\n");
    // GEMINI: 해당부분은 없어도 문제 없다.
    // 이유: C 언어 표준에 명시된 동작인 C 스타트업 코드가 BSS 영역 전체를 0으로 초기화하는 작업을 수행하기 때문인데, 
    //_os_ready_group과 _os_ready_table은 전역 변수라 BSS 영역에 위치하게 되고, 따라서 자동으로 0으로 초기화된다.
    // 하지만 유지시키기로 결정 (25/09/07-이종원)
    // 이유: 
    // 1. 코드의 명확성과 가독성을 위해 초기화 코드를 유지하는 것이 좋으므로 유지시킨다.
    // 2. C 스타트업 코드가 항상 존재하지 않음을 고려하여 유지시키자.
	
    /* Initializes ready_group */ //확인 완료 (25/09/07-이종원)
    _os_ready_group = 0; //8비트를 모두 0으로 초기화 //같은 파일 내 전역 변수로 선언

    /* Initializes ready_table */ //확인 완료 (25/09/07-이종원)
    for (int8u_t i = 0; i < READY_TABLE_SIZE; i++) {
        _os_ready_table[i] = 0; // 각 원소 (8 bits)를 0으로 초기화 //같은 파일 내 전역 변수로 선언
    }
}


int32u_t _os_get_highest_priority()
{
    int8u_t y = _os_unmap_table[_os_ready_group];

    return (int32u_t)((y << 3) + _os_unmap_table[_os_ready_table[y]]);
}


void _os_set_ready(int8u_t priority)
{
    /* Sets corresponding bit of ready_group to 1 */
    _os_ready_group |= _os_map_table[priority >> 3];

    /* Sets corresponding bit of ready_table to 1 */
    _os_ready_table[priority >> 3] |= _os_map_table[priority & 0x07];
}


void _os_unset_ready(int8u_t priority)
{
    /* Sets corresponding bit of ready_table to 0 */
    if ((_os_ready_table[priority >> 3] &= ~_os_map_table[priority & 0x07]) == 0) {
    /* If no ready task exists in the priority group,
	 * sets corresponding bit of ready_group to 0 */
        _os_ready_group &= ~_os_map_table[priority >> 3];
    }
}
