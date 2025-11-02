#include <core/eos.h>

void _os_spin_lock(_os_spinlock_t *lock)
{
    unsigned int tmp, val;

    do {
        do {
            __asm__ __volatile__(
                "ldaxr  %w0, [%1]\n"   // lock 값 읽기 + acquire barrier
                "cbnz   %w0, 1f\n"     // 0이 아니면 다른 코어가 점유 중
                "stxr   %w0, %w2, [%1]\n" // 0이면 시도
                "1:"
                : "=&r"(val), "+r"(lock), "=&r"(tmp)
                : "2"(SPINLOCK_LOCKED)
                : "memory");
        } while (val != 0 || tmp != 0); // 실패 시 반복
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
