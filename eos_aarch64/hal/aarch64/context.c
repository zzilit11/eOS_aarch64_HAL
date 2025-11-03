#include "type.h"
#include "uart.h"
#include "context.h"
#include <stddef.h>
/*
    Structure of AArch64 CPU context
        _os_context_t

    Context fucntions   
        1. print_context    
        2. _os_create_context
        3. _os_save_context
        4. _os_restore_context 
*/

static inline void *memset(void *dst, int val, size_t n) {
    int8u_t *p = dst;
    while (n--) *p++ = (int8u_t)val;
    return dst;
}

void print_context(addr_t ctx_addr)
{
    _os_context_t *ctx = (_os_context_t *)ctx_addr;
    // Debug output
    _os_serial_printf("[ctx] base=0x%lx\n", (int64u_t)ctx_addr);

    if (!ctx) {
        _os_serial_puts("Invalid context address\n");
        return;
    }

    _os_serial_puts("[Context dump]\n");
    for (int i = 0; i < 31; i++) {
        _os_serial_printf("x%d = 0x%lx\n", i, (int64u_t)ctx->x[i]);
    }

    _os_serial_printf("SP_EL0  = 0x%lx\n", (int64u_t)ctx->sp);
    _os_serial_printf("ELR_EL1 = 0x%lx\n", (int64u_t)ctx->elr_el1);
    _os_serial_printf("SPSR_EL1= 0x%lx\n", (int64u_t)ctx->spsr_el1);
    _os_serial_printf("=======================\n");
    
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg)
{
    // 1) 16B 정렬된 런타임 SP 계산
    int64u_t sp = ((int64u_t)stack_base + (int64u_t)stack_size) & ~0xFUL;

    // 2) 컨텍스트 프레임을 런타임 SP 아래에 배치
    _os_context_t *ctx = (_os_context_t *)(sp - sizeof(_os_context_t));

    // 3) 전체 0 초기화로 안전성 확보
    memset(ctx, 0, sizeof(*ctx));

     // 트램펄린 없이 첫 진입: x0=arg, ELR=entry
    ctx->x[0]     = (int64u_t)arg;      // entry의 첫 인자
    ctx->sp       = sp;         // 실제 태스크 SP (복원 시 이 값으로 mov sp)
    ctx->elr_el1  = (int64u_t)entry;    // 복귀 PC = entry
    ctx->spsr_el1 = 0x0000000000000005ULL;           // EL1h, 인터럽트 허용

    return (addr_t)ctx; // TCB->sp로
}
