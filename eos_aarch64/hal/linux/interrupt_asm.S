#********************************************************
#  Filename: hal/linux/interrupt_asm.S
# 
#  Author: Jiyong Park, RTOSLab. SNU
#  Modified by: Seongsoo Hong on 03/31/24
# 
#  Description: HAL routines for interrupt management
#********************************************************

#include "emulator_asm.h"

.text

/* Disables irq and return previous status */
.global hal_disable_interrupt
hal_disable_interrupt:
	mov _eflags, %eax
	_CLI
	ret

/* Enables irq by force */
.global hal_enable_interrupt
hal_enable_interrupt:
	_STI
	ret

/* Restores irq status */
.global hal_restore_interrupt
hal_restore_interrupt:
	mov 0x4(%esp), %eax
	mov %eax, _eflags
	ret
