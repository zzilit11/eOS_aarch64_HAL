/********************************************************
 * Filename: hal/linux/interrupt.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/28/24
 *
 * Description: HAL routines for irq management
 ********************************************************/

#include "emulator.h"
#include <core/eos_internal.h>


/**
 * Gets the highest-priority pending irq number
 * The higher the bit position is the lower the priority is
 */
// 얜 인자가 없네,, 왜지? 조건에 맞는 걸 구하는 느낌이네. 애초에 범용 함수라고도 했으니까
int32s_t hal_get_irq()
{
    for (int i = 0; i < 32; i++) {
        if (_irq_pending & ~_irq_mask & (0x1 << i)) {
            return i;
        }
    }

    return -1;
}


/**
 * Acknowledges the given irq
 * by clearing the corresponding bit in _irq_pending register
 */
void hal_ack_irq(int32u_t irq)
{
    _irq_pending &= ~(0x1 << irq); // irq의 비트만 지워버림. 
}


/* Masks the given irq */
void hal_disable_irq_line(int32u_t irq)
{
    _irq_mask |= (0x1 << irq);
}


/* Unmasks the given irq */
void hal_enable_irq_line(int32u_t irq)
{
    _irq_mask &= ~(0x1 << irq);
}
