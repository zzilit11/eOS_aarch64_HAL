#ifndef CONTEXT_H_
#define CONTEXT_H_

#define CTX_SIZE      272
#define CTX_OFF_SP    248
#define CTX_OFF_ELR   256
#define CTX_OFF_SPSR  264

#include "type.h"

typedef struct _os_context {
    int64u_t x[31];     // General purpose registers x0-x30
    int64u_t sp;        // Stack pointer
    int64u_t elr_el1;   // Exception Link Register (복귀 주소)
    int64u_t spsr_el1;  // Saved Program Status Register (복귀 시 PSTATE)
} _os_context_t;

void print_context(addr_t ctx_addr);

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg);

addr_t _os_save_context(void);
void   _os_restore_and_eret(addr_t ctx);
addr_t _os_save_context_el1(addr_t elr);

#endif // CONTEXT_H_