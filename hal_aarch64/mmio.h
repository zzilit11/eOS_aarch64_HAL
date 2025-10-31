#ifndef MMIO_H_
#define MMIO_H_
#include "type.h"

/* QEMU virt GICv2 base */
#define GICD_BASE   ((addr_t)(int64u_t)0x08000000)
#define GICC_BASE   ((addr_t)(int64u_t)0x08010000)


// Register read
static inline int32u_t mmio_read32(addr_t a) {
    return *(volatile int32u_t *)a;
}

// Register write
static inline void mmio_write32(addr_t a, int32u_t v) {
    *(volatile int32u_t *)a = v;
}



#endif  // MMIO_H_