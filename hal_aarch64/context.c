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

addr_t _os_create_context(addr_t stack_base,
                            size_t stack_size,
                            void (*entry)(void *),
                            void *arg)
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

/*
__attribute__((noreturn))
void _os_restore_context(addr_t sp)
{
    __asm__ volatile(
        // x9 = ctx*
        "mov    x9, %0\n\t"

        // SP/ELR/SPSR 먼저 복원
        "ldr    x10, [x9, #%c[OFF_SP]]\n\t"
        "ldr    x11, [x9, #%c[OFF_ELR]]\n\t"
        "ldr    x12, [x9, #%c[OFF_SPSR]]\n\t"

        "mov    sp, x10\n\t"
        "msr    ELR_EL1, x11\n\t"
        "msr    SPSR_EL1, x12\n\t"

        // GPR 복원 (x8,x9는 마지막에 개별 ldr로)
        "ldp    x0,  x1,  [x9, #0]\n\t"
        "ldp    x2,  x3,  [x9, #16]\n\t"
        "ldp    x4,  x5,  [x9, #32]\n\t"
        "ldp    x6,  x7,  [x9, #48]\n\t"
        // x8,x9 건너뛰기
        "ldp    x10, x11, [x9, #80]\n\t"
        "ldp    x12, x13, [x9, #96]\n\t"
        "ldp    x14, x15, [x9, #112]\n\t"
        "ldp    x16, x17, [x9, #128]\n\t"
        "ldp    x18, x19, [x9, #144]\n\t"
        "ldp    x20, x21, [x9, #160]\n\t"
        "ldp    x22, x23, [x9, #176]\n\t"
        "ldp    x24, x25, [x9, #192]\n\t"
        "ldp    x26, x27, [x9, #208]\n\t"
        "ldp    x28, x29, [x9, #224]\n\t"
        "ldr    x30,      [x9, #240]\n\t"   // LR

        // x8, x9 는 마지막에 단독 복원 (베이스 파괴 방지)
        "ldr    x8,       [x9, #64]\n\t"
        "ldr    x9,       [x9, #72]\n\t"

        "isb\n\t"
        "eret\n\t"
        :
        : "r"(sp),
          [OFF_SP]   "i"(31*8),         // 248
          [OFF_ELR]  "i"(31*8 + 8),     // 256
          [OFF_SPSR] "i"(31*8 + 16)     // 264
        : "x9","x10","x11","x12","memory"
    );
    __builtin_unreachable();
    
}


addr_t _os_save_context()
{
    addr_t ret;
    __asm__ volatile(
        // 컨텍스트 프레임 확보: sp' = sp - sizeof(_os_context_t)
        "sub    sp, sp, %c[SZ]\n\t"

        // x0~x30 저장 (SP 자체를 포인터로 활용)
        "stp    x0,  x1,  [sp, #(0*16)]\n\t"
        "stp    x2,  x3,  [sp, #(1*16)]\n\t"
        "stp    x4,  x5,  [sp, #(2*16)]\n\t"
        "stp    x6,  x7,  [sp, #(3*16)]\n\t"
        "stp    x8,  x9,  [sp, #(4*16)]\n\t"
        "stp    x10, x11, [sp, #(5*16)]\n\t"
        "stp    x12, x13, [sp, #(6*16)]\n\t"
        "stp    x14, x15, [sp, #(7*16)]\n\t"
        "stp    x16, x17, [sp, #(8*16)]\n\t"
        "stp    x18, x19, [sp, #(9*16)]\n\t"
        "stp    x20, x21, [sp, #(10*16)]\n\t"
        "stp    x22, x23, [sp, #(11*16)]\n\t"
        "stp    x24, x25, [sp, #(12*16)]\n\t"
        "stp    x26, x27, [sp, #(13*16)]\n\t"
        "stp    x28, x29, [sp, #(14*16)]\n\t"
        "str    x30,      [sp, #(15*16)]\n\t"

        // SP/ELR/SPSR 저장
        "add    x9, sp, %c[SZ]\n\t"          // x9 = 원래 sp
        "str    x9,       [sp, #%c[OFF_SP]]\n\t"
        "mrs    x10, ELR_EL1\n\t"
        "str    x10,      [sp, #%c[OFF_ELR]]\n\t"
        "mrs    x10, SPSR_EL1\n\t"
        "str    x10,      [sp, #%c[OFF_SPSR]]\n\t"

        // 호출자에게 컨텍스트 포인터 전달 후 원래 스택 복원
        "mov    %0, sp\n\t"
        "add    sp, sp, %c[SZ]\n\t"
        : "=r"(ret)
        : [SZ]      "i"(sizeof(_os_context_t)),
          [OFF_SP]  "i"(31*8),         // 248
          [OFF_ELR] "i"(31*8 + 8),     // 256
          [OFF_SPSR]"i"(31*8 + 16)     // 264
        : "x9","x10","memory"
    );
        return ret;
}


*/
