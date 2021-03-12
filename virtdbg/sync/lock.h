#ifndef __VIRTDBG_LOCK_H__
#define __VIRTDBG_LOCK_H__

#include <stdalign.h>
#include <stddef.h>

#define CACHELINE_SIZE (64)

typedef struct lock {
    alignas(CACHELINE_SIZE) size_t now_serving;
    alignas(CACHELINE_SIZE) size_t next_ticket;
} lock_t;

#define INIT_LOCK() ((lock_t){ 0 })

void lock(lock_t* lock);
void unlock(lock_t* lock);

#endif //__VIRTDBG_LOCK_H__
