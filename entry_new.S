/*
    entry.S
    1. Interrupt disable
    2. Exception level check and switch to EL1 if needed
    3. Stack pointer initialization
    4. Clear BSS section
    5. Set exception vector table address
    6. Call _os_initialization
*/

    .section .bss.trampoline, "aw", @nobits
    .align 11
_trampoline_stack:
    .skip 4096
_trampoline_stack_top:

    .section .text, "ax"
    .global _start
_start:
    /* 1. 모든 인터럽트 마스크 */
    msr     DAIFSet, #0xf

    /* 2. EL2 → EL1 강하(필요한 경우) */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2          // x0 = 현재 EL (0..3)
    cmp     x0, #2
    b.ne    1f                  // 이미 EL1이면 1으로 이동

    /* EL1을 AArch64로 실행 */
    mov     x1, #(1 << 31)      // HCR_EL2.RW = 1
    msr     HCR_EL2, x1

    /* EL1h로 복귀, DAIF 마스크 유지 */
    mov     x1, #0x3C5           // 0b00000000001111000101         // M[3:0]=0101 (EL1h)
    msr     SPSR_EL2, x1

    /* EL1에서 재개할 주소 지정 */
    adr     x1, 1f
    bic     x1, x1, #0b11        // 하위 2bit clear = 4B align
    isb
    msr     ELR_EL2, x1

    /* SP_EL1 설정 */
    ldr     x1, =__stack_top
    and     x1, x1, #-16
    msr     SP_EL1, x1

    /* Enable FP/NEON for EL1 ------------------------------------ */
    mrs     x0, CPACR_EL1
    orr     x0, x0, #(3 << 20)       // FPEN[21:20] = 11  → EL1에서 FP/SIMD 사용 허용
    msr     CPACR_EL1, x0
    isb
    /* ----------------------------------------------------------- */

    /* ================================================
    *  CNTP 타이머 EL1 접근 허용 (EL2 → EL1 전환 직전)
    * ================================================ */
    mrs     x0, CNTHCTL_EL2            // Read CNTHCTL_EL2
    orr     x0, x0, #(1 << 0)          // EL1PCTEN: CNTPCT_EL0 read 허용
    orr     x0, x0, #(1 << 1)          // EL1PCEN: CNTP_TVAL/CTL_EL0 접근 허용
    msr     CNTHCTL_EL2, x0            // 설정 반영
    msr     CNTKCTL_EL1, xzr           // EL0에서는 trap 안 함
    /* ================================================ */
    eret

1:
    /* 3. 스택 포인터 초기화 (EL1) */
    ldr     x0, =__stack_top
    and     x0, x0, #-16
    mov     sp, x0

    /* 4. BSS 클리어: [__bss_start, __bss_end) = 0
       링커에서 BSS 8바이트 정렬 보장 필요 */
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

    /* 유휴 루프 */
3:  wfe
    b       3b


/* ===========================
 * 하드웨어 예외 벡터 테이블 (2KiB 정렬)
 * 슬롯 간격 0x80
 * ===========================*/
    .section .vectors, "ax"
    .align  11
    .global vectors_el1
    .global __vectors_start
__vectors_start:
vectors_el1:
    /* Current EL with SP0 */
    b   el1_sync_sp0                 // +0x000
    .org vectors_el1 + 0x080
    b   el1_irq_sp0                  // +0x080
    .org vectors_el1 + 0x100
    b   el1_fiq_sp0
    .org vectors_el1 + 0x180
    b   el1_serr_sp0

    /* Current EL with SPx (일반 진입) */
    .org vectors_el1 + 0x200
    b   el1_sync_spx                 // +0x200
    .org vectors_el1 + 0x280
    b   el1_irq_spx                  // +0x280 ★ 사용할 IRQ 슬롯
    .org vectors_el1 + 0x300
    b   el1_fiq_spx
    .org vectors_el1 + 0x380
    b   el1_serr_spx

    /* Lower EL using AArch64 */
    .org vectors_el1 + 0x400
    b   el1_sync_el0
    .org vectors_el1 + 0x480
    b   el1_irq_el0
    .org vectors_el1 + 0x500
    b   el1_fiq_el0
    .org vectors_el1 + 0x580
    b   el1_serr_el0

    /* Lower EL using AArch32 */
    .org vectors_el1 + 0x600
    b   el1_sync_a32
    .org vectors_el1 + 0x680
    b   el1_irq_a32
    .org vectors_el1 + 0x700
    b   el1_fiq_a32
    .org vectors_el1 + 0x780
    b   el1_serr_a32


/* 슬롯 구현: 일단 대기. IRQ는 트램펄린 자리만 마련 */
    .section .text, "ax"
    .p2align 4

    .extern _os_common_interrupt_handler   // 또는 _os_irq_handler

el1_irq_spx:
    /* TODO: GICv3 ACK/EOI 연결
       mrs x0, ICC_IAR1_EL1     // x0=INTID
       bl  _os_common_interrupt_handler
       msr ICC_EOIR1_EL1, x0
       eret
    */
// 레지스터 저장

    // 1) 현재 SP(태스크/부트 스택)를 안전하게 보관
    mov     x19, sp                      // x19 = original SP (callee-saved라 C가 보존)

    // 2) 트램펄린 스택으로 교체 (16B 정렬 보장)
    ldr     x9, =_trampoline_stack_top
    and     x9, x9, #-16
    mov     sp, x9

    // GIC IAR 읽기 (IRQ ID)
    ldr  x1, =GICC_IAR    
    ldr  w0, [x1]           // w0 = IRQ ID

    mov     sp, x19
    
    // 4) C 핸들러: save→rearm→pick→EOI→restore(eret)
    bl   _os_common_interrupt_handler

    eret


/* 나머지 슬롯은 당분간 저전력 대기 */
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
