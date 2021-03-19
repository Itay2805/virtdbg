#include <stdbool.h>
#include <util/defs.h>
#include <util/except.h>
#include <util/string.h>
#include <sync/lock.h>
#include <stdint.h>
#include "pmm.h"

/**
 * The min order needs to fit a single pointer
 * for our linked list, meaning the min is 3
 */
#define MIN_ORDER 3
#define MAX_ORDER 18

#define NEXT(ptr) *((void**)(ptr))

/**
 * The base of the memory allocation
 */
static void* m_base = NULL;

/**
 * The free list of free blocks
 */
//static void* m_free_list[MAX_ORDER - MIN_ORDER] = { 0 };

/**
 * The PMM lock, to protect against multi-core allocations
 */
static lock_t m_pmm_lock = INIT_IRQ_LOCK();

void init_pmm(uintptr_t base, size_t size) {
    // align everything nicely
    uintptr_t aligned_base = ALIGN_UP(base, 4096);
    m_base = (void*)aligned_base;
} 

void* palloc_aligned(size_t size, size_t align) {
    lock(&m_pmm_lock);
    m_base = (void*)ALIGN_UP((size_t)m_base, align);
    void* res = m_base;
    m_base += size;
    unlock(&m_pmm_lock);
    return res;
}

void* palloc(size_t size) {
    return palloc_aligned(size, 1);
}

void* pallocz(size_t size) {
    void* ptr = palloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void* pallocz_aligned(size_t size, size_t align) {
    void* ptr = palloc_aligned(size, align);
    memset(ptr, 0, size);
    return ptr;
}

/*
static bool check_buddies(void* a, void* b, size_t size) {
    uintptr_t lower = MIN(a, b) - m_base;
    uintptr_t upper = MAX(a, b) - m_base;
    return (lower ^ size) == upper;
}

static void buddy_add_free_item(void* address, size_t order, bool new) {
    void* head = m_free_list[order - MIN_ORDER];
    NEXT(address) = 0;
    size_t size = 1ull << order;

    if (!new && head != 0) {
        void* prev = 0;
        while (true) {
            if (check_buddies(head, address, size)) {
                if (prev != 0) {
                    NEXT(prev) = NEXT(head);
                } else {
                    m_free_list[order - MIN_ORDER] = NEXT(head);
                }

                buddy_add_free_item(MIN(head, address), order + 1, false);
                break;
            }

            if (NEXT(head) == 0) {
                NEXT(head) = address;
                break;
            }

            prev = head;
            head = NEXT(head);
        }
    } else {
        // just put in the head
        NEXT(address) = head;
        m_free_list[order - MIN_ORDER] = address;
    }
}

void init_pmm(uintptr_t base, size_t size) {
    // align everything nicely
    uintptr_t aligned_base = ALIGN_UP(base, 4096);
    size = (base + size) - aligned_base;
    base = aligned_base;
    m_base = (void*)base;

    TRACE("Initializing PMM");
    TRACE("\tbase: 0x%p", m_base);
    TRACE("\tsize: %S", size);

    // as long as the chunk is big enough to fit
    while (size > (1ull << MIN_ORDER)) {

        // find the largest order and use that
        for (int order = MAX_ORDER - 1; order >= MIN_ORDER; order--) {
            if (size >= (1ull << order)) {
                buddy_add_free_item((void*)base, order, true);
                base += 1ull << order;
                size -= 1ull << order;
                break;
            }
        }
    }
}

void* palloc(size_t size) {
    TRACE("ALLOC %x", size);
    lock(&m_pmm_lock);

    int original_order = MAX(LOG2(size), MIN_ORDER);
    size_t want_size = 1ull << original_order;

    // make sure the buddy can actually do that
    if (original_order >= MAX_ORDER) {
        WARN("Tried to allocate too much from buddy (requested %zx bytes, order %d, max order %d)", size, original_order, MAX_ORDER);
        return NULL;
    }

    // find the smallest order with space
    for (int order = original_order; order < MAX_ORDER; order++) {
        TRACE("ORDER %x", order);
        if (m_free_list[order - MIN_ORDER] != 0) {
            // pop the head
            void* address = m_free_list[order - MIN_ORDER];
            m_free_list[order - MIN_ORDER] = NEXT(address);

            // try to free the left overs
            size_t found_size = 1ull << order;
            while (found_size > want_size) {
                found_size >>= 1ull;
                buddy_add_free_item(address + found_size, LOG2(found_size), true);
            }

            // return the allocated address
            unlock(&m_pmm_lock);
            return address;
        }
    }
    
    unlock(&m_pmm_lock);
    return NULL;
}

void pfree(void* ptr, size_t size) {
    lock(&m_pmm_lock);

    // get the order and add it
    int order = MAX(LOG2(size), MIN_ORDER);
    buddy_add_free_item(ptr, order, false);

    unlock(&m_pmm_lock);
}

void* pallocz(size_t size) {
    void* ptr = palloc(size);
    memset(ptr, 0, size);
    return ptr;
}*/
