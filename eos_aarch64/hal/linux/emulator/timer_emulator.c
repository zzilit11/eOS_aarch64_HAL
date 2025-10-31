/********************************************************
 * Filename: hal/linux/emulator/timer_emulator.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/27/24
 *
 * Description: Emulate the interval timer interrupt
 ********************************************************/

#include <core/eos.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "private.h"
// #include <newlib/sys/types.h>

static timer_t *_interval_timer;

/**
 * Signal handler
 */
static void sighandler(int signum, siginfo_t *info, void *context)
{
    // PRINT("generate irq (signum: %d, siginfo_t: %d, context: 0x%x)\n", signum, info->si_timerid, context);

    /* Generates an irq of 0, which is the irq number of the timer */
    _gen_irq(IRQ_INTERVAL_TIMER0);
}

/**
 * Initializes the interval timer
 */
void _init_timer_interrupt()
{
    /* Sets an handler for SIGALRM */
    struct sigaction action;
    action.sa_flags = SA_SIGINFO | SA_NODEFER;
    action.sa_sigaction = sighandler;
    if (sigaction(SIGALRM, &action, NULL) != 0)
    {
        return;
    }

    /* Creates an interval timer */
    if (timer_create(CLOCK_REALTIME, NULL, &_interval_timer) != 0)
    {
        return;
    }

    /* Runs the timer */
    struct timespec timer_period = {1, 0};
    // struct timespec timer_period1 = {0, 500000000};
    struct itimerspec timer_value = {timer_period, timer_period};
    if (timer_settime(_interval_timer, 0, &timer_value, NULL) != 0)
    {
        return;
    }
}
