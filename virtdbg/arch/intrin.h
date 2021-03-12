#ifndef __VIRTDBG_INTRIN_H__
#define __VIRTDBG_INTRIN_H__

#include <stdint.h>

typedef union ia32_rflags {
    struct {
        uint32_t CF:1;           ///< Carry Flag.
        uint32_t always_one:1;   ///< _reserved.
        uint32_t PF:1;           ///< Parity Flag.
        uint32_t _reserved_1:1;   ///< _reserved.
        uint32_t AF:1;           ///< Auxiliary Carry Flag.
        uint32_t _reserved_2:1;   ///< _reserved.
        uint32_t ZF:1;           ///< Zero Flag.
        uint32_t SF:1;           ///< Sign Flag.
        uint32_t TF:1;           ///< Trap Flag.
        uint32_t IF:1;           ///< Interrupt Enable Flag.
        uint32_t DF:1;           ///< Direction Flag.
        uint32_t OF:1;           ///< Overflow Flag.
        uint32_t IOPL:2;         ///< I/O Privilege Level.
        uint32_t NT:1;           ///< Nested Task.
        uint32_t _reserved_3:1;   ///< _reserved.
        uint32_t RF:1;           ///< Resume Flag.
        uint32_t VM:1;           ///< Virtual 8086 Mode.
        uint32_t AC:1;           ///< Alignment Check.
        uint32_t VIF:1;          ///< Virtual Interrupt Flag.
        uint32_t VIP:1;          ///< Virtual Interrupt Pending.
        uint32_t ID:1;           ///< ID Flag.
        uint32_t _reserved_4:10;  ///< _reserved.
    };
    uint64_t raw;
} __attribute__((packed)) ia32_rflags_t;

#endif //__VIRTDBG_INTRIN_H__
