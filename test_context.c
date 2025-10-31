#include "early_uart.h"
#include "interrupt.h"
#include "timer.h"
#include "context.h"
#include "mmio.h"

typedef struct {
    addr_t sp;
    int state;
} task_control_t;

static task_control_t tasks[2];
static int current_task = 0;


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
extern int32u_t read_cntp_tval_el0(void);

void dump_timer_registers(void) {
    early_uart_puts("[debug] CNTP_CTL_EL0=");
    int32u_t ctl = read_cntp_ctl_el0();
    _os_serial_printf("[CNTPC] CTL=%x (EN=%d IMASK=%d IST=%d)\n",
        ctl,
        (ctl>>0)&1, (ctl>>1)&1, (ctl>>2)&1);

    early_uart_puts("  CNTP_TVAL_EL0=");
    uart_put_hex(read_cntp_tval_el0(), 8);
    early_uart_putc('\n');
}




void dump_context_struct(const int tag, int id, addr_t sp) {
    early_uart_puts("[CTX] "); _os_serial_printf("%d\n", tag + 1);
    early_uart_puts(" task"); uart_put_hex(id, 1);
    early_uart_puts(" base="); uart_put_hex((int64u_t)sp, 8);
    early_uart_putc('\n');

    // 1) GPR x0..x30
    for (int i = 0; i < 31; i++) {
        early_uart_puts("x"); 
        // 숫자 자리 정렬
        if (i < 10) early_uart_putc('0' + i);
        else { early_uart_putc('1'); early_uart_putc('0' + (i - 10)); }
        early_uart_puts("  = 0x");
        uart_put_hex(*(int64u_t*)((char*)sp + 0 + i*8), 8);
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


void save_current_task_sp(addr_t sp) {
    tasks[current_task].sp = sp;
}

void debug_restore_context(addr_t sp, int task_id) {
    dump_context_struct(task_id, task_id, sp);
}

void validate_save_addr(addr_t saving_sp, int task_idx) {
    early_uart_puts("[CHECK] save_current_task_sp 인자 (saving_sp) = ");
    uart_put_hex((int64u_t)saving_sp, 8); early_uart_putc('\n');
    early_uart_puts("[CHECK] 현재 tasks[");
    early_uart_putc('0' + task_idx);
    early_uart_puts("].sp  = ");
    uart_put_hex((int64u_t)tasks[task_idx].sp, 8); early_uart_putc('\n');
}

// Interrupt handler
void _os_common_interrupt_handler(int32u_t irq)
{
    early_uart_puts("[ISR] Interrupt received: ");
    _os_serial_printf("IRQ=0x%x\n", irq);       
    early_uart_putc('\n');

    early_uart_puts("[ISR] Starting point\n");

    int32u_t id = (irq & 0x3FF);    // INTID 추출 (v2: 하위 10비트)
    _os_serial_printf("[ISR] IRQ ID: %d\n", id);

    if (id >= 1020) {
        return;   // 스푸리어스면 EOI도 치지 말고 바로 복귀
    }



    if (id == IRQ_CNTP) {
        // Save context
        debug_restore_context(tasks[current_task].sp, current_task);
        
        addr_t saved_sp = _os_save_context();
        _os_serial_printf("\n\n[ISR] Right after save_context");
        debug_restore_context(tasks[current_task].sp, current_task);
        early_uart_puts("\n\n");
        debug_restore_context(saved_sp, current_task);      
        
        save_current_task_sp(saved_sp);

        validate_save_addr(saved_sp, current_task);
        
        early_uart_puts("[ISR] timer rearm\n");
        _timer_rearm();

        current_task = (current_task + 1) % 2;

        early_uart_puts("[ISR] Switching to task ");
        early_uart_putc('0' + current_task + 1);
        early_uart_puts("\n");

        _os_serial_printf("\n\n eos_ack_irq's irq=0x%x\n", irq);       
        eos_ack_irq(irq);

        // Context 복구 (이후 dead code 불가)
        early_uart_puts("\n\n [ISR] Right before restore_context for task ");
        uart_put_hex(current_task, 1); 
        early_uart_putc('\n');
        debug_restore_context(tasks[current_task].sp, current_task);

        _os_restore_context(tasks[current_task].sp);

    } else {
         eos_ack_irq(irq);
    }
    // 여기는 도달하지 않음 (_os_restore_context가 eret하므로)
}

/********************************************************
 * 2. Task Entry Functions (테스트용)
 ********************************************************/
void task_func1(void *arg) {
    static int loop1 = 0;
    while (1) {
        early_uart_puts("\n\n\n===========================\n");
        early_uart_puts("[Task1] Running...\n");
        uart_put_hex(loop1++, 4); 
        early_uart_puts("===========================\n");

        _os_serial_printf("[Task%d]\n", current_task + 1);
        // debug_restore_context(tasks[current_task].sp, current_task);      
        
        __asm__ volatile("wfi");
        // wfi 이후
        early_uart_puts("[DBG] after wfi = ");        
        dump_timer_registers();

    }
}

void task_func2(void *arg) {
    static int loop2 = 0;
    while (1) {
        early_uart_puts("\n\n\n===========================\n");
        early_uart_puts("[Task2] Running... \n");
        uart_put_hex(loop2++, 4); 
        early_uart_puts("===========================\n");

        _os_serial_printf("[Task%d]\n", current_task + 1);
        // debug_restore_context(tasks[current_task].sp, current_task);

        __asm__ volatile("wfi");
        // wfi 이후
        early_uart_puts("[DBG] after wfi = ");        
        dump_timer_registers();
    }
}


// 초기화
extern void _os_restore_context(addr_t sp);


void _os_initialization(void) {

    early_uart_init(EARLY_UART_CLOCK_HZ);
    early_uart_puts("[boot] early uart up\n");
    _os_serial_puts("[boot] serial up\n");


    /* 이후 GIC, Generic Timer 등 HAL 초기화… */
    _gic_init();
    early_uart_puts("[boot] GIC initialized\n");

    _os_init_timer();
    early_uart_puts("[boot] Timer initialized\n");

    eos_enable_interrupt();
    early_uart_puts("[boot] Interrupts enabled\n");


    /* 컨텍스트 생성 */
    tasks[0].sp = _os_create_context((addr_t)0x40200000, 4096, task_func1, (void *)0x12345678);
    tasks[1].sp = _os_create_context((addr_t)0x40201000, 4096, task_func2, (void *)0x87654321);
    tasks[0].state = 1;  // Task1 먼저 실행
    tasks[1].state = 0;

    early_uart_puts("[boot] Contexts created\n");

    _os_serial_printf("[DBG] Task 1: ctx0=%x  \nx0(arg)=%lx  \nSP=%lx  \nELR=%lx  \nSPSR=%lx\n",
        (void*)tasks[0].sp,
        ((_os_context_t*)tasks[0].sp)->x[0],
        ((_os_context_t*)tasks[0].sp)->sp,
        ((_os_context_t*)tasks[0].sp)->elr_el1,
        ((_os_context_t*)tasks[0].sp)->spsr_el1);

    _os_serial_printf("[DBG] Task 2: ctx0=%x  \nx0(arg)=%lx  \nSP=%lx  \nELR=%lx  \nSPSR=%lx\n",
        (void*)tasks[1].sp,
        ((_os_context_t*)tasks[1].sp)->x[0],
        ((_os_context_t*)tasks[1].sp)->sp,
        ((_os_context_t*)tasks[1].sp)->elr_el1,
        ((_os_context_t*)tasks[1].sp)->spsr_el1);

    /* Task1 진입 */
    current_task = 0;
    _os_restore_context(tasks[0].sp);

    while (1);    
}
