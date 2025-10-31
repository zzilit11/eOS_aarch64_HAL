#include "type.h"
#include "mmio.h"
#include "interrupt.h"
#include "timer.h"
#include "uart.h"

/* Reload interval (tick period) */
static int32u_t _reload;

/* -------------------- Low-level register access -------------------- */
int64u_t read_cntfrq_el0(void)
{
    int64u_t val;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}

int32u_t read_cntp_ctl_el0(void)
{
    int64u_t val;
    __asm__ volatile("mrs %0, cntp_ctl_el0" : "=r"(val));
    return (int32u_t)val;
}

int32u_t read_cntp_tval_el0(void)
{
    int64u_t val;
    __asm__ volatile("mrs %0, cntp_tval_el0" : "=r"(val));
    return (int32u_t)val;
}

void write_cntp_tval_el0(int32u_t val)
{
    __asm__ volatile("msr cntp_tval_el0, %0" :: "r"((int64u_t)val) : "memory");
    asm volatile("isb");
}

void write_cntp_ctl_el0(int32u_t val)
{
    __asm__ volatile("msr cntp_ctl_el0, %0" :: "r"((int64u_t)val) : "memory");
    __asm__ volatile("isb");
}

/* -------------------- Public HAL functions -------------------- */

/* Re-arm the timer interrupt by reloading CNTP_TVAL_EL0 */
void _timer_rearm(void)
{
    write_cntp_tval_el0(_reload);
}

/* Initialize Generic Timer (CNTP) and enable its interrupt */
void _os_init_timer(void)
{
    int64u_t freq = read_cntfrq_el0();  // usually 62500000 on QEMU virt

    // 1초 틱(TICK_HZ=1) 기본. 나중에 TICK_HZ만 바꾸면 자동 반영됨.
    if ((int32u_t)TICK_HZ == 0u) {
        _reload = (int32u_t)freq;                  // 방어: 0이면 강제로 1Hz로
    } else {
        _reload = freq / (int32u_t)TICK_HZ;
        if (_reload == 0) 
            _reload = 1;   // 과도한 tick_hz 방어
    }                     // (1s per tick)

    // 1) 타이머 끄기
    write_cntp_ctl_el0(0);        // ENABLE=0, IMASK=don't care
    __asm__ volatile("isb");

     // 2) 초기 로드
    write_cntp_tval_el0(_reload); // 다음 인터럽트까지의 down-counter

    // 3) 타이머 켜기 + 마스크 해제
    // bit0 ENABLE=1, bit1 IMASK=0
    write_cntp_ctl_el0(1u);       // 명시적으로 1만 씀 = ENABLE=1, IMASK=0 (기본 0 가정)
    __asm__ volatile("isb");

    // 4) GIC에서 CNTP PPI 언마스크
    eos_enable_irq_line(IRQ_CNTP);


 
}