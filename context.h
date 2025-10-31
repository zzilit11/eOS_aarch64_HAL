#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "type.h"

typedef struct _os_context {
    int64u_t x[31];     // General purpose registers x0-x30
    int64u_t sp;      // Stack pointer
    int64u_t elr_el1;     // Exception Link Register (복귀 주소)
    int64u_t spsr_el1;    // Saved Program Status Register (복귀 시 PSTATE)
} _os_context_t;

void print_context(addr_t ctx_addr);

addr_t _os_create_context(addr_t stack_base,
                            size_t stack_size,
                            void (*entry)(void *),
                            void *arg);

addr_t _os_save_context(void);
void _os_restore_context(addr_t sp) __attribute__((noreturn));

#endif // CONTEXT_H_