#include <core/eos.h>

/* lock 획득 */
void _os_spin_lock(_os_spinlock_t *lock) {
    int tmp;
    do {
        __asm__ __volatile__(
            "xchg %0, %1"
            : "=r"(tmp), "=m"(*lock)
            : "0"(SPINLOCK_LOCKED), "m"(*lock)
            : "memory"
        );
        // tmp가 0이면 성공, 1이면 다른 코어가 이미 잡음 → 계속 반복
    } while (tmp != 0);
}

/* lock 해제 */
void _os_spin_unlock(_os_spinlock_t *lock) {
    __asm__ __volatile__(
        "movl $0, %0"
        : "=m"(*lock)
        :
        : "memory"
    );
}