/********************************************************
 * Filename: hal/linux/emulator/interrupt.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/27/24
 *
 * Description: Emulate interrupt-related registers
 *              and operations
 ********************************************************/

#include "private.h"

/**
 * Emulates only interrupt enable flag (IF) in eflags register
 *     eflags == 1: interrupt enabled, 0: interrupt disabled
 */
int32u_t _eflags = 1;
int32u_t _irq_pending = 0x0;
int32u_t _irq_mask = 0xFFFFFFFF;


void _cli()
{
    _eflags = 0;
}


void _sti()
{
    _eflags = 1;
    _deliver_irq();
}


/**
 * Emulation of interrupt generation and CPU reaction:
 *      Turns on _irq_pending bit and irq delivered to CPU
 */
void _gen_irq(int8u_t irq)
{
    _irq_pending |= (0x1 << irq); // irq로 가버린 1비트가 pending에 색칠
    _deliver_irq();
}
