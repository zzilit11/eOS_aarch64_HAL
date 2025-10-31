/********************************************************
 * Filename: hal/linux/init_hal.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/31/24
 *
 * Description: Initialize timer and enable its irq
 ********************************************************/

#include <core/eos.h>
#include "emulator.h"


/* Intializes hardware dependent parts */
void _os_init_hal()
{
    PRINT("Initializing hal module\n");

    /* Initializes timer interrupt */
    _init_timer_interrupt();

    /* Initiate interval timer by unmasking timer interrupt */
    hal_enable_irq_line(IRQ_INTERVAL_TIMER0);
}
