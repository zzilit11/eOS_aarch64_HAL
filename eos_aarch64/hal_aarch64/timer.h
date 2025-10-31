#ifndef TIMER_H_
#define TIMER_H_
#include "type.h"

#ifndef TICK_HZ
#define TICK_HZ 1u
#endif

/* CNTP PPI interrupt ID (GICv2 on ARMv8) */
#ifndef IRQ_CNTP
#define IRQ_CNTP 30
#endif

int64u_t read_cntfrq_el0(void);
int32u_t read_cntp_ctl_el0(void);
int32u_t read_cntp_tval_el0(void);

void write_cntp_tval_el0(int32u_t val);
void write_cntp_ctl_el0(int32u_t val);


/* Initialize the Generic Timer */
void _os_init_timer(void);

/* Rearm the timer for the next tick */
void _timer_rearm(void);

#endif  // TIMER_H_