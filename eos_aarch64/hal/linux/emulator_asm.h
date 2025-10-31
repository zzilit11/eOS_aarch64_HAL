/********************************************************
 * Filename: hal/linux/emulator_asm.h
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/31/24
 *
 * Description: Header for defining assembly instructions
 ********************************************************/

#ifndef HAL_LINUX_EMUL_ASM_H
#define HAL_LINUX_EMUL_ASM_H

/* Clear interrupt */
#define _CLI \
	movl $0, _eflags;

/* Set interrupt */
#define _STI \
	movl $1, _eflags;\
	call _deliver_irq;

/* Push eflags into stack */
#define _PUSHF \
	push _eflags;

/* Pop eflags from stack */
#define _POPF \
	pop _eflags;\
	call _deliver_irq;	

/**
 * Emulation of iret instruction (return from interrupt):
 * 	Check if there is a pending irq
 */
#define _IRET \
	call _deliver_irq;\
	ret;

#endif
