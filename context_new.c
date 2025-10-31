#include "type.h"
#include "early_uart.h"
#include "context.h"

/*
    Structure of AArch64 CPU context
        _os_context_t

    Context fucntions   
        1. print_context    
        2. _os_create_context
        3. _os_save_context
        4. _os_restore_context 
*/

void print_context(addr_t ctx_addr) {
    _os_context_t *ctx = (_os_context_t *)ctx_addr;

    _os_serial_printf("=======================\n");
    _os_serial_printf("[ctx] base=0x%lx\n", (int64u_t)ctx_addr);

    if (!ctx) {
        _os_serial_puts("Invalid context address\n");
        return;
    }

    _os_serial_puts("[Context dump]\n");
    for (int i = 0; i < 31; i++) {
        _os_serial_printf("x%d = 0x%lx\n", i, (int64u_t)ctx->x[i]);
    }

    _os_serial_printf("SP_EL0  = 0x%lx\n", (int64u_t)ctx->sp_el0);
    _os_serial_printf("ELR_EL1 = 0x%lx\n", (int64u_t)ctx->elr_el1);
    _os_serial_printf("SPSR_EL1= 0x%lx\n", (int64u_t)ctx->spsr_el1);
    _os_serial_printf("=======================\n\n");
}

addr_t _os_create_context(addr_t stack_base,
                            size_t stack_size,
                            void (*entry)(void *),
                            void *arg) {

        int64u_t *sp = (int64u_t *)((int64u_t)stack_base + stack_size);
        _os_context_t *ctx = (_os_context_t *)(sp - sizeof(_os_context_t) / sizeof(int64u_t));
    }


void _os_restore_context(addr_t sp);

addr_t _os_save_context(_os_context_t *ctx);
