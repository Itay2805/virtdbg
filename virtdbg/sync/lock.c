#include <stdatomic.h>

#include <util/except.h>
#include <arch/cpu.h>

#include "lock.h"

void lock(lock_t* lock) {
    DEBUG_ASSERT(lock != NULL, "Tried to lock a NULL lock");

    const size_t ticket = atomic_fetch_add_explicit(&lock->next_ticket, 1, memory_order_relaxed);
    while (atomic_load_explicit(&lock->now_serving, memory_order_acquire) != ticket) {
        cpu_pause();
    }
}

void unlock(lock_t* lock) {
    DEBUG_ASSERT(lock != NULL, "Tried to unlock a NULL lock");

    const size_t successor = atomic_load_explicit(&lock->now_serving, memory_order_relaxed) + 1;
    atomic_store_explicit(&lock->now_serving, successor, memory_order_release);
}
