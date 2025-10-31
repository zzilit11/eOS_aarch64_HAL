#ifndef CONTEXT_H_
#define CONTEXT_H_

// 어셈블리에서 사용할 오프셋 상수
#define CTX_SIZE sizeof(_os_context_t) // 272
#define CTX_OFF_SP   (31*8)            // 248
#define CTX_OFF_ELR  (31*8 + 8)        // 256
#define CTX_OFF_SPSR (31*8 + 16)       // 264

#include "type.h"

typedef struct _os_context {
    int64u_t x[31];     // General purpose registers x0-x30
    int64u_t sp;        // Stack pointer
    int64u_t elr_el1;   // Exception Link Register (복귀 주소)
    int64u_t spsr_el1;  // Saved Program Status Register (복귀 시 PSTATE)
} _os_context_t;

void print_context(addr_t ctx_addr);

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg);

#endif // CONTEXT_H_