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

// Save caller context into a stack frame and return its base in x0
addr_t _os_save_context(void) __attribute__((naked));
addr_t _os_save_context(void)
{
    __asm__ volatile(
        "sub     sp, sp, #%0\n"

        "stp     x0,  x1,  [sp, #(0 * 16)]\n"
        "stp     x2,  x3,  [sp, #(1 * 16)]\n"
        "stp     x4,  x5,  [sp, #(2 * 16)]\n"
        "stp     x6,  x7,  [sp, #(3 * 16)]\n"
        "stp     x8,  x9,  [sp, #(4 * 16)]\n"
        "stp     x10, x11, [sp, #(5 * 16)]\n"
        "stp     x12, x13, [sp, #(6 * 16)]\n"
        "stp     x14, x15, [sp, #(7 * 16)]\n"
        "stp     x16, x17, [sp, #(8 * 16)]\n"
        "stp     x18, x19, [sp, #(9 * 16)]\n"
        "stp     x20, x21, [sp, #(10 * 16)]\n"
        "stp     x22, x23, [sp, #(11 * 16)]\n"
        "stp     x24, x25, [sp, #(12 * 16)]\n"
        "stp     x26, x27, [sp, #(13 * 16)]\n"
        "stp     x28, x29, [sp, #(14 * 16)]\n"
        "str     x30,      [sp, #(15 * 16)]\n"

        "add     x1, sp, #%0\n"
        "str     x1,      [sp, #%1]\n"
        "mrs     x1, ELR_EL1\n"
        "str     x1,      [sp, #%2]\n"
        "mrs     x1, SPSR_EL1\n"
        "str     x1,      [sp, #%3]\n"

        "mov     x0, sp\n"
        "ret\n"
        :
        : "i"(CTX_SIZE),
          "i"(CTX_OFF_SP),
          "i"(CTX_OFF_ELR),
          "i"(CTX_OFF_SPSR)
        : "memory"
    );
}

// Restore from context pointer in x0 and eret
void _os_restore_and_eret(addr_t ctx) __attribute__((naked));
void _os_restore_and_eret(addr_t ctx)
{
    __asm__ volatile(
        // x0 = ctx
        "ldr     x1, [x0, #%0]\n"
        "ldr     x2, [x0, #%1]\n"
        "msr     ELR_EL1, x1\n"
        "msr     SPSR_EL1, x2\n"

        "ldp     x2,  x3,  [x0, #(1 * 16)]\n"
        "ldp     x4,  x5,  [x0, #(2 * 16)]\n"
        "ldp     x6,  x7,  [x0, #(3 * 16)]\n"
        "ldp     x8,  x9,  [x0, #(4 * 16)]\n"
        "ldp     x10, x11, [x0, #(5 * 16)]\n"
        "ldp     x12, x13, [x0, #(6 * 16)]\n"
        "ldp     x14, x15, [x0, #(7 * 16)]\n"
        "ldp     x16, x17, [x0, #(8 * 16)]\n"
        "ldp     x18, x19, [x0, #(9 * 16)]\n"
        "ldp     x20, x21, [x0, #(10 * 16)]\n"
        "ldp     x22, x23, [x0, #(11 * 16)]\n"
        "ldp     x24, x25, [x0, #(12 * 16)]\n"
        "ldp     x26, x27, [x0, #(13 * 16)]\n"
        "ldp     x28, x29, [x0, #(14 * 16)]\n"
        "ldr     x30,      [x0, #(15 * 16)]\n"

        "ldr     x3, [x0, #%2]\n"   // new sp

        "ldr     x1, [x0, #8]\n"
        "ldr     x0, [x0, #0]\n"

        "mov     sp, x3\n"
        "isb\n"
        "eret\n"
        :
        : "i"(CTX_OFF_ELR),
          "i"(CTX_OFF_SPSR),
          "i"(CTX_OFF_SP)
        : "memory"
    );
}

// Save caller context into a stack frame and return its base in x0
addr_t _os_save_context_el1(addr_t elr) __attribute__((naked));
addr_t _os_save_context_el1(addr_t elr)
{
    __asm__ volatile(
        "sub     sp, sp, #%0\n"

        // elr 인자(x0)를 context.elr에 먼저 기록
        "str     x0, [sp, #%2]\n"

        // GPR save
        "stp     x0,  x1,  [sp, #(0 * 16)]\n"
        "stp     x2,  x3,  [sp, #(1 * 16)]\n"
        "stp     x4,  x5,  [sp, #(2 * 16)]\n"
        "stp     x6,  x7,  [sp, #(3 * 16)]\n"
        "stp     x8,  x9,  [sp, #(4 * 16)]\n"
        "stp     x10, x11, [sp, #(5 * 16)]\n"
        "stp     x12, x13, [sp, #(6 * 16)]\n"
        "stp     x14, x15, [sp, #(7 * 16)]\n"
        "stp     x16, x17, [sp, #(8 * 16)]\n"
        "stp     x18, x19, [sp, #(9 * 16)]\n"
        "stp     x20, x21, [sp, #(10 * 16)]\n"
        "stp     x22, x23, [sp, #(11 * 16)]\n"
        "stp     x24, x25, [sp, #(12 * 16)]\n"
        "stp     x26, x27, [sp, #(13 * 16)]\n"
        "stp     x28, x29, [sp, #(14 * 16)]\n"
        "str     x30,      [sp, #(15 * 16)]\n"

        "add     x1, sp, #%0\n"
        "str     x1, [sp, #%1]\n"

        "mrs     x1, SPSR_EL1\n"
        "str     x1, [sp, #%3]\n"

        "mov     x0, sp\n"
        "ret\n"
        :
        : "i"(CTX_SIZE),
          "i"(CTX_OFF_SP),
          "i"(CTX_OFF_ELR),
          "i"(CTX_OFF_SPSR)
        : "memory"
    );
}