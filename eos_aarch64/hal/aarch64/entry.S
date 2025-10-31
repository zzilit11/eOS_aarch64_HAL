/*
    entry_new_context.S
    AArch64 startup + Inlined IRQ vector slot
*/

.section .text, "ax"
.global _start
_start:
    // Deactivate all exceptions
    msr     DAIFSet, #0xf 

    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    el1_entry
    mov     x1, #(1 << 31)
    msr     HCR_EL2, x1
    mov     x1, #0x3C5
    msr     SPSR_EL2, x1
    adr     x1, el1_entry
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

el1_entry:
    ldr     x0, =__stack_top
    and     x0, x0, #-16
    mov     sp, x0
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end

bss_clear_loop:  
    cmp     x1, x2
    b.hs    bss_clear_done
    str     xzr, [x1], #8
    b       bss_clear_loop

bss_clear_done:
    ldr     x0, =__vectors_start
    msr     VBAR_EL1, x0
    isb
    
os_init: 
    bl      _os_initialization

idle_loop:  
    wfe
    b       idle_loop

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
    b   el1_irq_spx_handler

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

/* =============================================
 * 실제 예외 핸들러 구현 (벡터 테이블 바깥)
 * ============================================= */

.equ CTX_SIZE, 272
.equ CTX_OFF_SP, 248
.equ CTX_OFF_ELR, 256
.equ CTX_OFF_SPSR, 264

el1_irq_spx_handler:                     
    // 1. 스택에 전체 컨텍스트 프레임 할당
    sub    sp, sp, #CTX_SIZE

    // 2. GPRs (x0-x30) 저장
    stp    x0,  x1,  [sp, #(0*16)]
    stp    x2,  x3,  [sp, #(1*16)]
    stp    x4,  x5,  [sp, #(2*16)]
    stp    x6,  x7,  [sp, #(3*16)]
    stp    x8,  x9,  [sp, #(4*16)]
    stp    x10, x11, [sp, #(5*16)]
    stp    x12, x13, [sp, #(6*16)]
    stp    x14, x15, [sp, #(7*16)]
    stp    x16, x17, [sp, #(8*16)]
    stp    x18, x19, [sp, #(9*16)]
    stp    x20, x21, [sp, #(10*16)]
    stp    x22, x23, [sp, #(11*16)]
    stp    x24, x25, [sp, #(12*16)]
    stp    x26, x27, [sp, #(13*16)]
    stp    x28, x29, [sp, #(14*16)]
    str    x30,      [sp, #(15*16)]     // 240

    // 3. SP, ELR, SPSR 저장
    // (스크래치 레지스터로 x0, x1 사용. 이미 저장했으므로 안전)
    add    x0, sp, #CTX_SIZE            // x0 = 원래 sp
    str    x0,      [sp, #CTX_OFF_SP]
    mrs    x0, ELR_EL1
    str    x0,      [sp, #CTX_OFF_ELR]
    mrs    x0, SPSR_EL1
    str    x0,      [sp, #CTX_OFF_SPSR]

    // 4. C 핸들러 호출 준비
    ldr    x1, =0x0801000C          // x1 = GIC_IAR 주소
    ldr    w0, [x1]                 // w0 = irq 번호 (C 인자 1)
    mov    x1, sp                   // x1 = context_ptr (C 인자 2)

    // 5. Call C handler 
    bl   _os_common_interrupt_handler

    // 6. C 핸들러 리턴 -> 공용 복원 함수로 점프 (꼬리 호출)
    // x0에 이미 복원할 SP가 들어있으므로, _os_restore_and_eret로 점프합니다.
    b      _os_restore_and_eret


.global _os_restore_and_eret
_os_restore_and_eret:
mov    x9, x0                   // x9 = 복원할 컨텍스트 포인터
// 7. SP/ELR/SPSR 복원 (시스템 레지스터 먼저)
ldr    x10, [x9, #CTX_OFF_ELR]
ldr    x11, [x9, #CTX_OFF_SPSR]
msr    ELR_EL1, x10
msr    SPSR_EL1, x11
// 8. GPRs 복원
ldp    x0,  x1,  [x9, #(0*16)]
ldp    x2,  x3,  [x9, #(1*16)]
ldp    x4,  x5,  [x9, #(2*16)]
ldp    x6,  x7,  [x9, #(3*16)]
// x8, x9는 베이스 레지스터(x9)와 겹치므로 나중에
ldp    x10, x11, [x9, #(5*16)]
ldp    x12, x13, [x9, #(6*16)]
ldp    x14, x15, [x9, #(7*16)]
ldp    x16, x17, [x9, #(8*16)]
ldp    x18, x19, [x9, #(9*16)]
ldp    x20, x21, [x9, #(10*16)]
ldp    x22, x23, [x9, #(11*16)]
ldp    x24, x25, [x9, #(12*16)]
ldp    x26, x27, [x9, #(13*16)]
ldp    x28, x29, [x9, #(14*16)]
ldr    x30,     [x9, #(15*16)]
// 9. SP 및 x8, x9 복원 (마지막)
ldr    x8,      [x9, #(4*16)]     // x8 복원
ldr    x10,     [x9, #CTX_OFF_SP] // x10 = 복원할 SP
ldr    x9,      [x9, #(4*16 + 8)] // x9 복원 (x10을 스크래치로 사용)
mov    sp, x10                  // SP 복원
// 10. 예외 복귀
isb
eret

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
