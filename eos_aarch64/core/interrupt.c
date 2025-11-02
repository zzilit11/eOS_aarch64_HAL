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
    }
}


static void copy_context(_os_context_t *dst, const _os_context_t *src)
{
    if (!dst || !src) {
        return;
    }

    int64u_t *dst_words = (int64u_t *)dst;
    const int64u_t *src_words = (const int64u_t *)src;
    const size_t word_count = sizeof(_os_context_t) / sizeof(int64u_t);
    const int64u_t dst_addr = (int64u_t)dst;
    const int64u_t src_addr = (int64u_t)src;

    if (dst_addr == src_addr) {
        return;
    }

    if (dst_addr > src_addr && dst_addr < src_addr + sizeof(_os_context_t)) {
        for (size_t i = word_count; i-- > 0;) {
            dst_words[i] = src_words[i];
        }
        return;
    }

    for (size_t i = 0; i < word_count; ++i) {
        dst_words[i] = src_words[i];
    }
}

void save_current_task_sp(addr_t sp)
{
    eos_tcb_t *tcb = eos_get_current_task();
    if (!tcb) {
        return;
    }

    _os_context_t *dest = (_os_context_t *)tcb->sp;
    const _os_context_t *src = (const _os_context_t *)sp;

    if (!dest) {
        return;
    }

    copy_context(dest, src);
}

// aarch64
void _os_common_interrupt_handler(int32u_t irq, addr_t saved_context_ptr) {
    /* Gets irq number */
    int32u_t irq_num = hal_get_irq();
    
    /* Acknowledges the irq */
    hal_ack_irq(irq_num);

    /* Restores _eflags */
    hal_restore_interrupt(flag); 

    /* Dispatches the handler and call it */
    _os_icb_t *p = &_os_icb_table[irq_num];
    if (p->handler != NULL) {
        save_current_task_sp(saved_context_ptr);
        p->handler(irq_num, p->arg); // timer_interrupt_handler 호출
    }
    return tasks[current_task].sp;
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
