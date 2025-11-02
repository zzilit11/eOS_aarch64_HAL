#include "uart.h"
#include "interrupt.h"
#include "timer.h"
#include "context.h"
#include "mmio.h"

__attribute__((noreturn))
void _os_restore_and_eret(addr_t sp);

extern int32u_t read_cntp_ctl_el0(void); // 이미 있으면 바로 사용
extern int32u_t read_cntp_tval_el0(void);


void dump_context_struct(const int tag, int id, addr_t sp) {
    early_uart_puts("[CTX] "); _os_serial_printf("%d\n", tag + 1);
    early_uart_puts(" task"); uart_put_hex(id, 1);
    early_uart_puts(" base="); uart_put_hex((int64u_t)sp, 8);
    early_uart_putc('\n');

    // 1) GPR x0..x30
    for (int i = 0; i < 31; i++) {
        early_uart_puts("x");
        if (i < 10) {
            early_uart_putc('0' + i);
        } else {
            int tens = i / 10;
            int ones = i % 10;
            early_uart_putc('0' + tens);
            early_uart_putc('0' + ones);
        }
        early_uart_puts("  = 0x");
        uart_put_hex(*(int64u_t *)((char *)sp + i * 8), 8); // 기존 규격 유지
        early_uart_putc('\n');
    }

    // 2) SP(EL1), ELR_EL1, SPSR_EL1 — 저장과 동일 오프셋!
    int64u_t sp_el1  = *(int64u_t*)((char*)sp + 31 * 8);
    int64u_t elr_el1 = *(int64u_t*)((char*)sp + 31 * 8 + 8);
    int64u_t spsr_el1= *(int64u_t*)((char*)sp + 31 * 8 + 16);

    _os_serial_printf("SP(EL1) = 0x%lx\n", sp_el1);
    _os_serial_printf("ELR_EL1 = 0x%lx\n", elr_el1);
    _os_serial_printf("SPSR_EL1= 0x%lx\n", spsr_el1);

}

void debug_restore_context(addr_t sp, int task_id) {
    dump_context_struct(task_id, task_id, sp);
}