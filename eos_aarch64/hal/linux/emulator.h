/********************************************************
 * Filename: hal/linux/emulator.h
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/27/24
 *
 * Description: Header for Linux emulator
 ********************************************************/

#ifndef HAL_LINUX_EMUL_H
#define HAL_LINUX_EMUL_H

#include "type.h"

/* eOS currently supports only the interval timer 0 */
#define IRQ_INTERVAL_TIMER0 0

/* start address of vector table 
   _vector[0]: reset vector
   _vector[1]: reserved
   _vector[2]: reserved
   _vector[3]: irq vector */
extern int32u_t _vector[];

/* At reset, the following steps are performed. 
  1. Disable interrupts
  2. Set %esp to the bottom of _os_init_stack
  3. Jump to _vector[0]
*/

/* At irq, the following steps are performed. 
  1. Disalbe interrupts
  2. Push %eip
  3. Push %eflags
  4. Jump to _vector[3]
*/

/* irq pending register
   If n-th bit is 1, irq n is pending
   If user resets n-th bit, the corresponding irq is acknowledged
*/
extern int32u_t _irq_pending;

/* irq mask register
   If n-th bit is set to 1, irq n is masked
   If n-th bit is set to 0, irq n is unmasked
   Intially, all irqs are masked
*/
extern int32u_t _irq_mask;

void _init_timer_interrupt();

#endif
