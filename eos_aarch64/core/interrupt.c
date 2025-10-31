/********************************************************
 * Filename: core/interrupt.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/30/24
 *
 * Description: User interface for handling interrupts
 ********************************************************/

#include <core/eos.h>

/**
 * ICB structure that represents an in-kernel status of an irq
 */
typedef struct icb { // 확인 완료(25/09/07-이종원)
    int8s_t irqnum;				// irq number
    void (*handler)(int8s_t irqnum, void *arg);	// the handler function //table에 시레로 호출될 함수의 주소를 저장
    void *arg;   // argument given to the handler when interrupt occurs
} _os_icb_t;

/**
 * Table of ICBs for all interrupts
 */
_os_icb_t _os_icb_table[IRQ_MAX]; //Interrupt Control Block이 IQR_MAX크기로 테이블에 저장된 형태


void _os_init_icb_table() // 확인 완료(25/09/07-이종원)
{
    PRINT("Initializing interrupt module\n");

    for (int8s_t i = 0; i < IRQ_MAX; i++) {
        _os_icb_t *p = &_os_icb_table[i];
        p->irqnum = i;
        p->handler = NULL;
        p->arg = NULL; 
        // 핸들러에 전달될 arg인자도 NULL로 초기화
        // 없어도 전혀 문제 없으나, 코드 가독성 관점을 위해 추가
        // 없어도 문제 없는 이유: _os_common_interrupt_handler에서 핸들러가 NULL인지 체크 후 호출하기 때문
        // 추가 (25/09/07-이종원) 
    }
}


void _os_common_interrupt_handler(int32u_t flag)
{
    /* Gets irq number */
    int32u_t irq_num = hal_get_irq();
    if (irq_num == -1)
        return;
	
    /* Acknowledges the irq */
    hal_ack_irq(irq_num);
	
    /* Restores _eflags */
    hal_restore_interrupt(flag); 

    /* Dispatches the handler and call it */
    _os_icb_t *p = &_os_icb_table[irq_num];
    if (p->handler != NULL) {
        //PRINT("entering irq handler %p\n", (void*)(p->handler));
        p->handler(irq_num, p->arg);
        //PRINT("exiting irq handler %p\n", (void*)(p->handler));
    }
}


int8s_t eos_set_interrupt_handler(int8s_t irqnum, eos_interrupt_handler_t handler, void *arg)
{
    /* Validate parameters: irq range and optional handler (NULL = unregister) */
    if (irqnum < 0 || irqnum >= IRQ_MAX) {
        PRINT("invalid irqnum=%d\n", (int32u_t)irqnum);
        return -1; /* EINVAL */
    }

    PRINT("irqnum: %d, handler: %p, arg: %p\n", (int32u_t)irqnum, (void*)handler, arg);

    _os_icb_t *p = &_os_icb_table[irqnum];
    p->handler = handler; /* NULL means unregister */
    p->arg = arg;

    return 0;
}
// 확인 완료(25/09/14-이종원)


eos_interrupt_handler_t eos_get_interrupt_handler(int8s_t irqnum)
{
    if (irqnum < 0 || irqnum >= IRQ_MAX) {
        PRINT("eos_get_interrupt_handler: invalid irqnum=%d\n", (int32u_t)irqnum);
        return NULL;
    }

    _os_icb_t *p = &_os_icb_table[irqnum];
    return p->handler;
}	
