#include "src/lib/spinlock.h"

void spinlock_acquire(spinlock_t *lock) {
    for (;;) {
        if (spinlock_test_and_acq(lock)) {
            break;
        }
    }
    return;
}