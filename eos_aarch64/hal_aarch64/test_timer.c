#include "uart.h"
#include "interrupt.h"
#include "timer.h"
#include "context.h"

// 타이머 인터럽트 카운터
static volatile int timer_interrupt_count = 0;

void dump_vbar_register(void) {
    int64u_t vbar_val;
    __asm__ volatile ("mrs %0, VBAR_EL1" : "=r" (vbar_val));
    early_uart_puts("[debug] VBAR_EL1=");
    uart_put_hex(vbar_val, 16);   // 16자리 HEX로 출력
    early_uart_putc('\n');
}

void dump_daif_register(void) {
    int64u_t daif_val;
    __asm__ volatile ("mrs %0, DAIF" : "=r" (daif_val));
    early_uart_puts("[debug] DAIF=");
    uart_put_hex(daif_val, 8);
    early_uart_putc('\n');
}

extern int32u_t read_cntp_ctl_el0(void); // 이미 있으면 바로 사용
void dump_timer_registers(void) {
    early_uart_puts("[debug] CNTP_CTL_EL0=");
    uart_put_hex(read_cntp_ctl_el0(), 8);
    early_uart_putc('\n');
}

// Interrupt handler
void _os_common_interrupt_handler(int32u_t irq)
{
    early_uart_puts("[irq] handler entered!\n");

    if (irq == IRQ_CNTP) {
        timer_interrupt_count++;
        _timer_rearm();
    }
}



// 초기화
extern void _os_restore_context(addr_t sp);

void _os_initialization(void) {

    
    early_uart_init(EARLY_UART_CLOCK_HZ);
    early_uart_puts("[boot] early uart up\n");
    _os_serial_puts("[boot] serial up\n");
    
    dump_vbar_register();
    dump_daif_register();
    dump_timer_registers();


    /* 이후 GIC, Generic Timer 등 HAL 초기화… */
    _gic_init();
    early_uart_puts("[boot] GIC initialized\n");

    _os_init_timer();
    early_uart_puts("[boot] Timer initialized\n");

    early_uart_puts("[boot] Before Interrupts enabled\n");
    dump_daif_register();
    eos_enable_interrupt();
    early_uart_puts("[boot] Interrupts enabled\n");

    dump_daif_register();
    dump_timer_registers();

     /****************************************************
     * 타이머 인터럽트 테스트
     ****************************************************/
    early_uart_puts("[boot] Waiting for timer interrupts...\n");
    early_uart_puts("[boot] Timer interrupt count will be displayed\n");

    int last_count = 0;

    // 무한 루프에서 타이머 인터럽트 카운터 모니터링
    while (1) {
        // WFI (Wait For Interrupt) 명령어로 인터럽트 대기
        __asm__ volatile("wfi");
        
        // 인터럽트 발생 후 카운터 확인
        if (timer_interrupt_count != last_count) {
            last_count = timer_interrupt_count;
            early_uart_puts("[main] Tick ");
            early_uart_puts("\n");
        }
        
    }

    early_uart_puts("[boot] Halting...\n");
    while (1) {
        __asm__ volatile("wfi");
    }
    
}
