#include <core/eos.h>

void _os_spin_lock(_os_spinlock_t *lock)
{
    unsigned int loaded, status, one = SPINLOCK_LOCKED;

    do {
        do {
            __asm__ __volatile__(
                "ldaxr  %w[loaded], [%[addr]]\n"   // loaded = *lock
                "cbnz   %w[loaded], 1f\n"          // 이미 잠김이면 건너뛰기
                "stxr   %w[status], %w[one], [%[addr]]\n" // 시도, status=0이면 성공
                "1:"
                : [loaded]"=&r"(loaded), [status]"=&r"(status)
                : [addr]"r"(lock), [one]"r"(one)
                : "memory");
        } while (loaded != 0 || status != 0);
    } while (0);
}


void _os_spin_unlock(_os_spinlock_t *lock)
{
    __asm__ __volatile__(
        "stlr  wzr, [%0]\n"   // 0 저장 + release barrier
        :
        : "r"(lock)
        : "memory");
}
