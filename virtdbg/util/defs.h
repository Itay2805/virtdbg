#ifndef __VIRTDBG_DEFS_H__
#define __VIRTDBG_DEFS_H__

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))

#define LOG2(x) ((unsigned) (64ull - __builtin_clzll(x) - 1ull))

#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#endif //__VIRTDBG_DEFS_H__
