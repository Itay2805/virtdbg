#ifndef __VIRTDBG_INTRIN_H__
#define __VIRTDBG_INTRIN_H__

#include <stdint.h>
#include <virtdbg.h>

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

uint64_t __rdmsr(uint32_t msr);

void __wrmsr (uint32_t msr, uint64_t Value);

typedef union {
    struct {
        uint32_t  VME:1;          ///< Virtual-8086 Mode Extensions.
        uint32_t  PVI:1;          ///< Protected-Mode Virtual Interrupts.
        uint32_t  TSD:1;          ///< Time Stamp Disable.
        uint32_t  DE:1;           ///< Debugging Extensions.
        uint32_t  PSE:1;          ///< Page Size Extensions.
        uint32_t  PAE:1;          ///< Physical Address Extension.
        uint32_t  MCE:1;          ///< Machine Check Enable.
        uint32_t  PGE:1;          ///< Page Global Enable.
        uint32_t  PCE:1;          ///< Performance Monitoring Counter
        ///< Enable.
        uint32_t  OSFXSR:1;       ///< Operating System Support for
        ///< FXSAVE and FXRSTOR instructions
        uint32_t  OSXMMEXCPT:1;   ///< Operating System Support for
        ///< Unmasked SIMD Floating Point
        ///< Exceptions.
        uint32_t  UMIP:1;         ///< User-Mode Instruction Prevention.
        uint32_t  LA57:1;         ///< Linear Address 57bit.
        uint32_t  VMXE:1;         ///< VMX Enable.
        uint32_t  SMXE:1;         ///< SMX Enable.
        uint32_t  _reserved3:1;   ///< _reserved.
        uint32_t  FSGSBASE:1;     ///< FSGSBASE Enable.
        uint32_t  PCIDE:1;        ///< PCID Enable.
        uint32_t  OSXSAVE:1;      ///< XSAVE and Processor Extended States Enable.
        uint32_t  _reserved4:1;   ///< _reserved.
        uint32_t  SMEP:1;         ///< SMEP Enable.
        uint32_t  SMAP:1;         ///< SMAP Enable.
        uint32_t  PKE:1;          ///< Protection-Key Enable.
        uint32_t  _reserved5:9;   ///< _reserved.
    };
    uint64_t raw;
} ia32_cr4_t;

typedef union {
    struct {
        uint32_t  PE:1;           ///< Protection Enable.
        uint32_t  MP:1;           ///< Monitor Coprocessor.
        uint32_t  EM:1;           ///< Emulation.
        uint32_t  TS:1;           ///< Task Switched.
        uint32_t  ET:1;           ///< Extension Type.
        uint32_t  NE:1;           ///< Numeric Error.
        uint32_t  _reserved0:10;  ///< _reserved.
        uint32_t  WP:1;           ///< Write Protect.
        uint32_t  _reserved1:1;   ///< _reserved.
        uint32_t  AM:1;           ///< Alignment Mask.
        uint32_t  _reserved2:10;  ///< _reserved.
        uint32_t  NW:1;           ///< Mot Write-through.
        uint32_t  CD:1;           ///< Cache Disable.
        uint32_t  PG:1;           ///< Paging.
    };
    uint64_t raw;
} ia32_cr0_t;

#define MSR_CODE_IA32_FEATURE_CONTROL 0x3A

typedef union ia32_feature_control {
    struct {
        uint64_t LockBit : 1;	    // 0		If the lock bit is clear, an attempt to execute
                                    //			VMXON will cause a #GP fault
        uint64_t VmxInSmx : 1;		// 1		Enables VMX in SMX operation
        uint64_t VmxOutsideSmx : 1;	// 2		Enables VMX outside SMX operation
        uint64_t reserved0 : 5;		// 3-7
        uint64_t SenterLocals : 7;	// 8-14		Enabled functionality of the SENTER leaf function
        uint64_t SenterGlobal : 1;	// 15		Global enable of all SENTER functionalities
        uint64_t reserved1 : 48;		// 16-63
    };
    uint64_t raw;
} ia32_feature_control_t;

ia32_cr4_t __readcr4(void);
uint64_t __readcr3(void);
ia32_cr0_t __readcr0(void);
void __writecr0(ia32_cr0_t data);
void __writecr4(ia32_cr4_t Data);
descriptor_t __sgdt();
descriptor_t __sidt();
void __lgdt(descriptor_t gdt);
void __lidt(descriptor_t idt);

#endif //__VIRTDBG_INTRIN_H__
