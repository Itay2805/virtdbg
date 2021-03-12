#ifndef __VIRTDBG_EPT_H__
#define __VIRTDBG_EPT_H__

#include <util/except.h>

err_t init_ept();

/**
 * Map the given address to the guest, we only do identity mapping
 * with rwx permissions
 */
err_t ept_map(uintptr_t address);

#endif //__VIRTDBG_EPT_H__
