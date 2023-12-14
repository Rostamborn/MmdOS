#ifndef SPIN_LOCK_DEF
#define SPIN_LOCK_DEF

#include <stdbool.h>

#ifndef SPINLOCK_T_DEF
typedef struct {
    int lock;
} spinlock_t;
#define SPINLOCK_T_DEF
#endif

#define SPINLOCK_INIT                                                          \
    { 0 }

static inline bool spinlock_test_and_acq(spinlock_t *lock) {
    return __sync_bool_compare_and_swap(&lock->lock, 0, 1);
}

void spinlock_acquire(spinlock_t *lock);

static inline void spinlock_release(spinlock_t *lock) {
    __atomic_store_n(&lock->lock, 0, __ATOMIC_SEQ_CST);
}

#endif