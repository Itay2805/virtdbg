#ifndef __VIRTDBG_LOCK_H__
#define __VIRTDBG_LOCK_H__

#include <stdatomic.h>
#include <stdbool.h>

typedef struct lock {
    size_t now_serving;
    size_t next_ticket;
} lock_t;

#define INIT_LOCK() ((lock_t){ 0 })

/**
 * Try to take the lock, if already taken then wait for the lock to be
 * freed and then try to take it
 *
 * @remark
 * This will raise the TPL of the owner to the TPL of the lock, once the lock is
 * released the original TPL will be released
 *
 * @param lock  [IN] The lock to take
 */
void acquire_lock(lock_t* lock);

/**
 * Try to acquire the lock, if failed will simply return false, otherwise return true
 *
 * @param lock  [IN] The lock to take
 */
bool acquire_lock_or_fail(lock_t* lock);

/**
 * Releases a taken lock, this will also restore the original TPL of the taker
 *
 * @param lock  [IN] The lock to release
 */
void release_lock(lock_t* lock);

#endif //__VIRTDBG_LOCK_H__
