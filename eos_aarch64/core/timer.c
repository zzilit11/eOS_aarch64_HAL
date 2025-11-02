/********************************************************
 * Filename: core/timer.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/30/24
 *
 * Description: User interface for handling interrupts
 ********************************************************/

#include <core/eos.h>

static eos_counter_t system_timer;
// OS 내부적으로 사용하는 시스템 타이머 카운터
// 확인 완료 (25/09/07-이종원)


int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value)
{
    if (counter == NULL) return 1; // 유효성 검사 추가 (25/09/07-이종원)
    counter->tick = init_value;
    counter->alarm_queue = NULL;

    return 0;
}
// 수정 완료 (25/09/07-이종원)


void eos_set_alarm(eos_counter_t *counter, eos_alarm_t *alarm, int32u_t timeout, void (*entry)(void *arg), void *arg)
{
    /* Validate inputs */
    if (counter == NULL || alarm == NULL) {
        PRINT("eos_set_alarm: invalid counter(%p) or alarm(%p)\n", (void*)counter, (void*)alarm);
        return;
    }

    /* Removes the alarm from the counter if it exists in the counter */
    _os_remove_node(&(counter->alarm_queue), &(alarm->queue_node));

    /* No more new alarm when timeout is 0 or entry is NULL */
    if (timeout == 0 || entry == NULL) return ;

    /* Prepares a new alarm */
    alarm->timeout = counter->tick + timeout;
    alarm->handler = entry;
    alarm->arg = arg;
    alarm->queue_node.pnode = (void*) alarm;
    alarm->queue_node.order_val = alarm->timeout;
    
    /* Adds the new alarm to the counter */
    _os_add_node_ordered(&(counter->alarm_queue), &(alarm->queue_node));

}


eos_counter_t *eos_get_system_timer()
{
    return &system_timer;
}


void eos_trigger_counter(eos_counter_t *counter)
{
    if (counter == NULL) {
        PRINT("eos_trigger_counter: counter is NULL\n");
        return;
    }

    counter->tick++; // eos_trigger_counter의 핵심 동작 1: tick 1 증가

    // Print the current time in ticks
    if (counter == &system_timer) {
        PRINT("------------------------\n");
        PRINT("system clock: %d\n", counter->tick);
    }

    if (counter->alarm_queue) { // eos_trigger_counter의 핵심 동작 2: 알람 큐에 등록된 알람들 중에서 만료된 것들을 처리
        eos_alarm_t * alarm = (eos_alarm_t *) counter->alarm_queue->pnode;
        while (alarm) {
            if (alarm->timeout > counter->tick) {
                break; // 아직 만료되지 않은 알람이므로 반복문 종료
            }
            // 여기까지 왔다면, 해당 알람은 만료된 것이므로, 알람 큐에서 제거 (pop)
            _os_remove_node(&counter->alarm_queue, &alarm->queue_node); 
            alarm->handler(alarm->arg); // 알람이 만료되었으므로, 알람의 handler 함수 호출
            if (counter->alarm_queue) { // 다음 알람이 존재한다면, 다음 알람을 가져옴
                alarm = (eos_alarm_t *) counter->alarm_queue->pnode;
            } else { // 다음 알람이 존재하지 않는다면, 반복문 종료
                alarm = NULL;
            }
        }
    }
    eos_schedule();
}


/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg)
{
    /* Triggers alarms */
    _timer_rearm(); // Reload the timer
    eos_trigger_counter(&system_timer);
}


void _os_init_timer()
{
    PRINT("Initializing timer module\n");

    eos_init_counter(&system_timer, 0);
    // 시스템 전역 변수인 system_timer를 초기화 시킴

    /* Registers timer interrupt handler */
    eos_set_interrupt_handler(IRQ_CNTP, timer_interrupt_handler, NULL);
    // IRQ_CNTP: ARM 아키텍처에서 제공하는 기본 timer interrupt 번호
}
