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
// 입력으로 받은 timer counter 구조체를 초기화
// OS 내부적으로는 해당 파일 내 _os_init_timer()함수에서 system_timer라는 하나의 전역 변수를 초기화 시킴
// 사용자가 직접 사용할 수 있으므로, 유효성 검사를 수행할 수 있도록 수정
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

    // Print the current time in ticks // 만약 System Timer라면, 현재 tick 값을 출력
    // 사용자가 직접 생성한 timer counter라면 출력하지 않음
    if (counter == &system_timer) {
        PRINT("------------------------\n");
        PRINT("system clock: %d\n", counter->tick);
    }

    if (counter->alarm_queue) { // eos_trigger_counter의 핵심 동작 2: 알람 큐에 등록된 알람들 중에서 만료된 것들을 처리
        eos_alarm_t * alarm = (eos_alarm_t *) counter->alarm_queue->pnode;
        while (alarm) {
            if (alarm->timeout > counter->tick) break; // 아직 만료되지 않은 알람이므로 반복문 종료
            // 오름차순 정렬된 연결리스트이므로, timeout이 더 큰 알람들은 당연히 만료되지 않으므로, break
            _os_remove_node(&counter->alarm_queue, &alarm->queue_node); // 여기까지 왔다면, 해당 알람은 만료된 것이므로, 알람 큐에서 제거 (pop)
            // _os_remove_node 함수에서 해당 노드에 연결된 다음 노드의 주소를 counter->alarm_queue에 넣어준다. (pop 처리를 해준다)
            // 따라서, 다음 반복문에서는 다음 알람을 처리하게 된다.
            // 만약, 다음 알람이 존재하지 않는다면, alarm은 NULL이 될 것이다.
            // 확인 완료 (25/09/14-이종원)
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
    eos_trigger_counter(&system_timer);
    // H/W의 timer interrupt가 발생했을 때, OS가 어떤 동작을 해야하는 지 설정하는 부분임
    // 현재는 timer interrupt가 발생했을 때, system_timer를 트리거하도록 설정되어 있고,
    // 이는 system_timer의 tick이 증가하고, 만료된 알람들이 처리됨을 의미함
    // 확인 완료 (25/09/14-이종원)
}


void _os_init_timer()
{
    PRINT("Initializing timer module\n");

    eos_init_counter(&system_timer, 0);
    // 시스템 전역 변수인 system_timer를 초기화 시킴
    // 함수이름의 접두사가 eos인 이유는 사용자도 직접 eos_counter_t 구조체를 사용하여 timer counter를 생성할 수 있고,
    // 이를 초기화시키는 함수를 사용자에게 제공하기 위함임. 즉, naming에 문제없음
    // 확인 완료 (25/09/07-이종원) 

    /* Registers timer interrupt handler */
    eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
    // timer interrupt handler를 등록하는 함수 호출
    // IRQ_INTERVAL_TIMER0는 hal/emulator.h에 0으로 정의되어 있음: 해당 H/W 인터럽트가 발생했을 때, OS가 어떤 동작을 해야하는 지 설정함
    // 여기서부터 진행해야함, 진행 중: eos (25/09/07-이종원)
}
