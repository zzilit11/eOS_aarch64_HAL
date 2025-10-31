/********************************************************
 * Filename: hal/linux/emulator/private.h
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/27/24
 *
 * Description: Internal header for Linux emulator
 ********************************************************/

#ifndef HAL_LINUX_EMUL_PRIVATE_H
#define HAL_LINUX_EMUL_PRIVATE_H

#include "../type.h"

extern int32u_t _eflags;
extern int32u_t _eflags_saved;

void _gen_irq(int8u_t irq);
void _deliver_irq();

#endif
