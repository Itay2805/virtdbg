#include <stdatomic.h>

#include <util/except.h>
#include <arch/cpu.h>

#include "lock.h"

void lock(lock_t* lock) {
    if (lock->disable_interrupts) {
        lock->interrupts = are_interrupts_enabled();
        disable_interrupts();
    }
    const size_t ticket = atomic_fetch_add_explicit(&lock->next_ticket, 1, memory_order_relaxed);
    while (atomic_load_explicit(&lock->now_serving, memory_order_acquire) != ticket) {
        if (lock->disable_interrupts && lock->interrupts) {
            enable_interrupts();
        }
        cpu_pause();
        disable_interrupts();
    }
}

void unlock(lock_t* lock) {
    const size_t successor = atomic_load_explicit(&lock->now_serving, memory_order_relaxed) + 1;
    atomic_store_explicit(&lock->now_serving, successor, memory_order_release);

    // restore interrupts
    if (lock->disable_interrupts && lock->interrupts) {
        enable_interrupts();
    }
}
