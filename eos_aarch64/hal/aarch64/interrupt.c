#include "type.h"
#include "mmio.h"
#include "interrupt.h"

/* -------------------- Internal state -------------------- */
static volatile int32u_t irq;

/* -------------------- Initialization -------------------- */
void _gic_init(void)
{
    // 1. Disable Distributor and CPU Interface during config
    mmio_write32((GICC_CTLR), 0x0);
    mmio_write32((GICD_CTLR), 0x0);

    // 2. Accept all priorities & no preemption split
    mmio_write32((GICC_PMR), 0xFF);
    mmio_write32((GICC_BPR), 0x0);

    // 3. Route all SPIs to CPU0 (PPIs는 해당 없음)
    for (int reg = 8; reg < 24; reg++) {
        mmio_write32((GICD_ITARGETSR) + reg * 4, 0x01010101);
    }

    
    // 4. Enable Distributor and CPU interface
    //    여기서 EOImodeNS=0을 "확실히" 강제: bit9=0, EnableGrp0(bit0)=1
    //    (기존에 0x1만 쓰던 거에서 bit9를 명시적으로 클리어)
    int32u_t ctl = mmio_read32((GICC_CTLR));
    ctl &= ~GICC_CTLR_EOIMODENS;   // EOImodeNS=0 강제
    ctl |= 0x3u;                   // EnableGrp0
    mmio_write32((GICC_CTLR), ctl);

    mmio_write32((GICD_CTLR), 0x3u);
}

/* -------------------- IRQ line control -------------------- */
void hal_enable_irq_line(int32s_t irq)
{
    addr_t reg = (GICD_ISENABLER0 + (irq / 32) * 4);
    mmio_write32(reg, 1u << (irq % 32));
}

void hal_disable_irq_line(int32s_t irq)
{
    addr_t reg = (GICD_ICENABLER0 + (irq / 32) * 4);
    mmio_write32(reg, 1u << (irq % 32));
}

/* -------------------- CPU Interrupt control -------------------- */
void hal_enable_interrupt(void)
{
    __asm__ volatile ("msr daifclr, #2" ::: "memory");
    __asm__ volatile ("isb" ::: "memory");
}

int64u_t hal_disable_interrupt(void)
{
    int64u_t prev;
    __asm__ volatile("mrs %0, daif" : "=r" (prev) :: "memory");
    __asm__ volatile ("msr daifset, #2" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
    return prev;
}

void hal_restore_interrupt(int64u_t flag)
{
    __asm__ volatile("msr DAIF, %0" :: "r"(flag) : "memory");
    __asm__ volatile("isb" ::: "memory");
}

/* -------------------- IRQ acknowledge & EOI -------------------- */
int32u_t hal_get_irq(void)
{
    irq = mmio_read32((GICC_IAR));
    int id = irq & 0x3FF;
    if (id >= 1020)  // spurious
        return -1;
    return id;
}

/* EOI handling aligned with EOImodeNS setting */
void hal_ack_irq(int32u_t irq)
{
    // EOIR: write the raw IAR value as-is
    mmio_write32((addr_t)(int64u_t)GICC_EOIR, (int32u_t)irq);

    // DIR: always write once more regardless of EOImodeNS (needed especially in QEMU)
    int32u_t intid = (int32u_t)irq & 0x3FFu;
    mmio_write32((addr_t)(int64u_t)GICC_DIR, intid);
}


/* -------------------- Compatibility stub -------------------- */
void _deliver_irq(void)
{
    __asm__ volatile ("dsb sy; isb");
}
