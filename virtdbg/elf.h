#ifndef __VIRTDBG_ELF_H__
#define __VIRTDBG_ELF_H__

#include <stdint.h>

typedef struct elf64_rela {
    uintptr_t offset;
    uint64_t info;
    int64_t addend;
} elf64_rela_t;

#define ELF64_R_SYM(i)			((i) >> 32)
#define ELF64_R_TYPE(i)			((i) & 0xffffffff)
#define ELF64_R_INFO(sym,type)		((((Elf64_Xword) (sym)) << 32) + (type))

#define R_X86_64_NONE		0
#define R_X86_64_RELATIVE	8

#endif //__VIRTDBG_ELF_H__
