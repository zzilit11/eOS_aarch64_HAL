#ifndef INTERRUPT_H_
#define INTERRUPT_H_
#include "type.h"
#include "mmio.h"

/* QEMU virt GICv2 base */

#define GICC_CTLR        (addr_t)(int64u_t)(GICC_BASE + 0x000)  /* Control */
#define GICC_PMR         (addr_t)(int64u_t)(GICC_BASE + 0x004)  /* Priority mask */
#define GICC_BPR         (addr_t)(int64u_t)(GICC_BASE + 0x008)  /* Binary point */
#define GICC_IAR         (addr_t)(int64u_t)(GICC_BASE + 0x00C)  /* Interrupt Ack */
#define GICC_EOIR        (addr_t)(int64u_t)(GICC_BASE + 0x010)  /* End of IRQ */
#define GICC_RPR         (addr_t)(int64u_t)(GICC_BASE + 0x014)  /* Running priority */
#define GICC_HPPIR       (addr_t)(int64u_t)(GICC_BASE + 0x018)  /* Highest pend prio */
#define GICC_ABPR        (addr_t)(int64u_t)(GICC_BASE + 0x01C)
#define GICC_AIAR        (addr_t)(int64u_t)(GICC_BASE + 0x020)  /* Secure alias (unused) */
#define GICC_AEOIR       (addr_t)(int64u_t)(GICC_BASE + 0x024)
#define GICC_AHPPIR      (addr_t)(int64u_t)(GICC_BASE + 0x028)
#define GICC_APR0        (addr_t)(int64u_t)(GICC_BASE + 0x0D0)
#define GICC_NSAPR0      (addr_t)(int64u_t)(GICC_BASE + 0x0E0)
#define GICC_IIDR        (addr_t)(int64u_t)(GICC_BASE + 0x0FC)
#define GICC_DIR         (addr_t)(int64u_t)(GICC_BASE + 0x1000) /* Deactivate (EOImode split에서 사용) */

/* EOImodeNS bit (GICv2, Non-secure) */
#define GICC_CTLR_EOIMODENS   (1u << 9)

/* ----- Distributor (GICD) ----- */
#define GICD_CTLR        (addr_t)(int64u_t)(GICD_BASE + 0x000)  /* Control */

/* Banked SGI/PPI registers: 0..31 → *0 를 명시 */
#define GICD_IGROUPR0    (addr_t)(int64u_t)(GICD_BASE + 0x080)
#define GICD_ISENABLER0  (addr_t)(int64u_t)(GICD_BASE + 0x100)
#define GICD_ICENABLER0  (addr_t)(int64u_t)(GICD_BASE + 0x180)
#define GICD_ISPENDR0    (addr_t)(int64u_t)(GICD_BASE + 0x200)
#define GICD_ICPENDR0    (addr_t)(int64u_t)(GICD_BASE + 0x280)
#define GICD_ISACTIVER0  (addr_t)(int64u_t)(GICD_BASE + 0x300)
#define GICD_ICACTIVER0  (addr_t)(int64u_t)(GICD_BASE + 0x380)

/* SPI targeting (byte-addressed blocks) */
#define GICD_ITARGETSR   (addr_t)(int64u_t)(GICD_BASE + 0x800)

/* Priority registers (byte-addressed: index = INTID) */
#define GICD_IPRIORITYR  (addr_t)(int64u_t)(GICD_BASE + 0x400)

static inline int _gic_eoimode_split(void)
{
    // 0: EOIR만 쓰면 끝, 1: EOIR 후 DIR=INTID 필요
    int32u_t ctlr = mmio_read32((addr_t)(int64u_t)GICC_CTLR);
    return (ctlr & GICC_CTLR_EOIMODENS) ? 1 : 0;
}

void _gic_init(void);
void hal_enable_irq_line(int32s_t irq);  
void hal_disable_irq_line(int32s_t irq);
void hal_enable_interrupt(void);
int64u_t hal_disable_interrupt(void);
void hal_restore_interrupt(int64u_t flag);

int32u_t hal_get_irq(void);
void hal_ack_irq(int32u_t irq);
void _deliver_irq(void);

#endif // INTERRUPT_H_