/*
    entry_new_context.S
    AArch64 startup + Inlined IRQ vector slot
*/

.section .text, "ax"
.global _start
_start:
    /* 1. 모든 인터럽트 마스크 */
    msr     DAIFSet, #0xf

    /* 2. EL2 → EL1 강하(필요한 경우) */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    1f
    mov     x1, #(1 << 31)
    msr     HCR_EL2, x1
    mov     x1, #0x3C5
    msr     SPSR_EL2, x1
    adr     x1, 1f
    bic     x1, x1, #0b11
    isb
    msr     ELR_EL2, x1
    ldr     x1, =__stack_top
    and     x1, x1, #-16
    msr     SP_EL1, x1
    mrs     x0, CPACR_EL1
    orr     x0, x0, #(3 << 20)
    msr     CPACR_EL1, x0
    isb
    mrs     x0, CNTHCTL_EL2
    orr     x0, x0, #(1 << 0)
    orr     x0, x0, #(1 << 1)
    msr     CNTHCTL_EL2, x0
    msr     CNTKCTL_EL1, xzr
    eret

1:
    ldr     x0, =__stack_top
    and     x0, x0, #-16
    mov     sp, x0
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
0:  cmp     x1, x2
    b.hs    2f
    str     xzr, [x1], #8
    b       0b
2:
    /* 5. 예외 벡터 테이블 등록 */
    ldr     x0, =__vectors_start
    msr     VBAR_EL1, x0
    isb
    
    /* 6. C 초기화 진입 */
    bl      _os_initialization
3:  wfe
    b       3b

/* ===========================
 * 예외 벡터 테이블 (2KiB align)
 * ===========================*/
.section .vectors, "ax"
.align  11
.global vectors_el1
.global __vectors_start
__vectors_start:
vectors_el1:
    /* SP0/EL1 slots (30%는 stub) */
    b   el1_sync_sp0                // +0x000
    .org vectors_el1 + 0x080
    b   el1_irq_sp0                 // +0x080
    .org vectors_el1 + 0x100
    b   el1_fiq_sp0
    .org vectors_el1 + 0x180
    b   el1_serr_sp0

    .org vectors_el1 + 0x200
    b   el1_sync_spx                // +0x200

    .org vectors_el1 + 0x280
el1_irq_spx:                        // +0x280 (중요! INLINE 핸들러 코드)

    stp  x0, x1, [sp, #-32]!
    stp  x2, x3, [sp, #16]
    str  x30, [sp, #-16]!
    ldr  x1, =0x0801000C
    ldr  w0, [x1]
    str  x0, [sp, #-16]!
    bl   _os_common_interrupt_handler
    ldr  x0, [sp], #16
    ldr  x1, =0x08010010
    str  w0, [x1]
    ldr  x30, [sp], #16
    ldp  x2, x3, [sp, #16]
    ldp  x0, x1, [sp], #32
    eret

    .org vectors_el1 + 0x300
    b   el1_fiq_spx
    .org vectors_el1 + 0x380
    b   el1_serr_spx

    .org vectors_el1 + 0x400
    b   el1_sync_el0
    .org vectors_el1 + 0x480
    b   el1_irq_el0
    .org vectors_el1 + 0x500
    b   el1_fiq_el0
    .org vectors_el1 + 0x580
    b   el1_serr_el0

    .org vectors_el1 + 0x600
    b   el1_sync_a32
    .org vectors_el1 + 0x680
    b   el1_irq_a32
    .org vectors_el1 + 0x700
    b   el1_fiq_a32
    .org vectors_el1 + 0x780
    b   el1_serr_a32

/* 모든 나머지 벡터 슬롯은 stub (필요시 수정)
 * (예시) */
el1_sync_sp0:  wfe; b el1_sync_sp0
el1_irq_sp0:   wfe; b el1_irq_sp0
el1_fiq_sp0:   wfe; b el1_fiq_sp0
el1_serr_sp0:  wfe; b el1_serr_sp0

el1_sync_spx:  wfe; b el1_sync_spx
el1_fiq_spx:   wfe; b el1_fiq_spx
el1_serr_spx:  wfe; b el1_serr_spx

el1_sync_el0:  wfe; b el1_sync_el0
el1_irq_el0:   wfe; b el1_irq_el0
el1_fiq_el0:   wfe; b el1_fiq_el0
el1_serr_el0:  wfe; b el1_serr_el0

el1_sync_a32:  wfe; b el1_sync_a32
el1_irq_a32:   wfe; b el1_irq_a32
el1_fiq_a32:   wfe; b el1_fiq_a32
el1_serr_a32:  wfe; b el1_serr_a32

/* 필요에 따라 추가 핸들러나 최상위 stub 함수를 C와 연결 */
.extern _os_common_interrupt_handler
