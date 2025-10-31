#********************************************************
#  Filename: hal/linux/entry.S
# 
#  Author: Jiyong Park, RTOSLab. SNU
#  Modified by: Seongsoo Hong on 03/28/24
# 
#  Description: Construct the interrupt vector table
#********************************************************

#include "emulator_asm.h"

# Fills the interrupt vector table
.data
.global _vector
_vector:
	.long _os_reset_handler
	.long _os_reset_handler
	.long _os_reset_handler
	.long _os_irq_handler

# New stack for eOS
.global _os_init_stack_start
.global _os_init_stack_end
_os_init_stack_start:
	.skip 4096
_os_init_stack_end:


.text
.global _os_reset_handler
_os_reset_handler:
	_CLI
	lea _os_init_stack_end, %esp
	call _os_init
	# Never reach here


.global _os_irq_handler
_os_irq_handler:
	pusha
	push _eflags_saved
	call _os_common_interrupt_handler
	add $0x4,%esp
	popa
	_IRET
