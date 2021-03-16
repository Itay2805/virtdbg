#ifndef __VIRTDBG_VMM_H__
#define __VIRTDBG_VMM_H__
#include <stdbool.h>
#include <util/except.h>
#include <virtdbg.h>

#define MSR_IA32_VMX_PINBASED_CTLS         0x00000481
#define MSR_IA32_VMX_PROCBASED_CTLS2       0x0000048B
#define PIN_BASED_VM_EXEC_CONTROLS         0x00004000
#define MSR_IA32_VMX_PROCBASED_CTLS        0x00000482
#define PROC_BASED_VM_EXEC_CONTROLS        0x00004002
#define PROC_BASED_VM_EXEC_CONTROLS2       0x0000401e
#define EXCEPTION_BITMAP                   0x00004004
#define VM_EXIT_CONTROLS                   0x0000400c
#define MSR_IA32_VMX_EXIT_CTLS             0x00000483
#define VM_EXIT_HOST_ADDR_SPACE_SIZE       0x00000200
#define VM_ENTRY_CONTROLS                  0x00004012
#define MSR_IA32_VMX_ENTRY_CTLS            0x00000484
#define VM_ENTRY_IA32E_MODE                0x00000200
#define HOST_CR0                           0x00006c00
#define HOST_CR3                           0x00006c02
#define HOST_CR4                           0x00006c04
#define HOST_ES_SELECTOR                   0x00000c00
#define HOST_CS_SELECTOR                   0x00000c02
#define HOST_SS_SELECTOR                   0x00000c04
#define HOST_DS_SELECTOR                   0x00000c06
#define HOST_FS_SELECTOR                   0x00000c08
#define HOST_GS_SELECTOR                   0x00000c0a
#define HOST_TR_SELECTOR                   0x00000c0c
#define HOST_FS_BASE                       0x00006c06
#define HOST_GS_BASE                       0x00006c08
#define HOST_TR_BASE                       0x00006c0a
#define HOST_GDTR_BASE                     0x00006c0c
#define HOST_IDTR_BASE                     0x00006c0e
#define HOST_IA32_SYSENTER_ESP             0x00006c10
#define HOST_IA32_SYSENTER_EIP             0x00006c12
#define HOST_IA32_SYSENTER_CS              0x00004c00
#define HOST_RSP                           0x00006c14
#define HOST_RIP                           0x00006c16
#define RFLAG_RESERVED                     (1 << 1)
#define GUEST_RFLAG                        0x00006820
#define HOST_GDT_LIMIT                     14 * 8

#define IA32_VMX_BASIC_MSR                 0x480
#define IA32_VMX_CR0_FIXED0_MSR            0x486
#define IA32_VMX_CR0_FIXED1_MSR            0x487
#define IA32_VMX_CR4_FIXED0_MSR            0x488
#define IA32_VMX_CR4_FIXED1_MSR            0x489
#define IA32_VMX_PINBASED_CTLS_MSR         0x481
#define IA32_VMX_PRI_PROCBASED_CTLS_MSR    0x482
#define IA32_VMX_SEC_PROCBASED_CTLS_MSR    0x48b
#define IA32_VMX_EPT_VPID_CAP_MSR          0x48c
#define IA32_VMX_VM_EXIT_CTLS_MSR          0x483
#define IA32_VMX_VM_ENTRY_CTLS_MSR         0x484
#define HOST_EFER_FULL                     0x00002c02

#define GUEST_DR7  0x0000681A
#define GUEST_RSP  0x0000681C
#define GUEST_RIP  0x0000681E
#define GUEST_CR0  0x00006800
#define GUEST_CR3  0x00006802
#define GUEST_CR4  0x00006804
#define CTLS_EPTP  0x0000201A

#define GUEST_ES_SELECTOR                 0x00000800
#define GUEST_CS_SELECTOR                 0x00000802
#define GUEST_SS_SELECTOR                 0x00000804
#define GUEST_DS_SELECTOR                 0x00000806
#define GUEST_FS_SELECTOR                 0x00000808
#define GUEST_GS_SELECTOR                 0x0000080a
#define GUEST_LDTR_SELECTOR               0x0000080c
#define GUEST_TR_SELECTOR                 0x0000080e
#define GUEST_ES_LIMIT                    0x00004800
#define GUEST_CS_LIMIT                    0x00004802
#define GUEST_SS_LIMIT                    0x00004804
#define GUEST_DS_LIMIT                    0x00004806
#define GUEST_FS_LIMIT                    0x00004808
#define GUEST_GS_LIMIT                    0x0000480a
#define GUEST_LDTR_LIMIT                  0x0000480c
#define GUEST_TR_LIMIT                    0x0000480e
#define GUEST_GDTR_LIMIT                  0x00004810
#define GUEST_IDTR_LIMIT                  0x00004812
#define GUEST_ES_AR_BYTES                 0x00004814
#define GUEST_CS_AR_BYTES                 0x00004816
#define GUEST_SS_AR_BYTES                 0x00004818
#define GUEST_DS_AR_BYTES                 0x0000481a
#define GUEST_FS_AR_BYTES                 0x0000481c
#define GUEST_GS_AR_BYTES                 0x0000481e
#define GUEST_LDTR_AR_BYTES               0x00004820
#define GUEST_TR_AR_BYTES                 0x00004822
#define GUEST_ES_BASE                     0x00006806
#define GUEST_CS_BASE                     0x00006808
#define GUEST_SS_BASE                     0x0000680a
#define GUEST_DS_BASE                     0x0000680c
#define GUEST_FS_BASE                     0x0000680e
#define GUEST_GS_BASE                     0x00006810
#define GUEST_LDTR_BASE                   0x00006812
#define GUEST_TR_BASE                     0x00006814
#define GUEST_GDTR_BASE                   0x00006816
#define GUEST_IDTR_BASE                   0x00006818

#define GUEST_ES_ACCESS_RIGHT         0x00004814
#define GUEST_CS_ACCESS_RIGHT         0x00004816
#define GUEST_SS_ACCESS_RIGHT         0x00004818
#define GUEST_DS_ACCESS_RIGHT         0x0000481A
#define GUEST_FS_ACCESS_RIGHT         0x0000481C
#define GUEST_GS_ACCESS_RIGHT         0x0000481E
#define GUEST_LDTR_ACCESS_RIGHT       0x00004820
#define GUEST_TR_ACCESS_RIGHT         0x00004822
#define GUEST_INTERRUPTIBILITY_STATE  0x00004824
#define GUEST_SMBASE                  0x00004828
#define GUEST_IA32_SYSENTER_CS        0x0000482A
#define GUEST_VMX_PREEMPTION_TIMER    0x0000482E
#define VMCS_FIELD_GUEST_EFER_FULL    0x00002806
#define MSR_FS_BASE                   0xc0000100
#define MSR_GS_BASE                   0xc0000101
#define EFER                          0xc0000080

#define GUEST_ACTIVITY_STATE               0X00004826
#define VMX_PREEMPTION_TIMER_VALUE         0x0000482E
#define VMCS_LINK_POINTER                  0x00002800
#define GUEST_INTR_STATUS                  0x00000810
#define GUEST_PML_INDEX                    0x00000812
#define VM_EXIT_REASON                     0x00004402
#define VM_INSTRUCTION_ERROR               0x00004400
#define EPT_VIOLATION_ADDRESS              0x00002400
#define EPT_VIOLATION_FLAGS                0x00006400

#define DATA_ACCESS_RIGHT  (0x3 | 1 << 4 | 1 << 7)
#define CODE_ACCESS_RIGHT  (0x3 | 1 << 4 | 1 << 7 | 1 << 13)
#define LDTR_ACCESS_RIGHT  (0x2 | 1 << 7)
#define TR_ACCESS_RIGHT    (11 | 1 << 7)

#define VMEXIT_EXTERNAL_INTERRUPT            1
#define VMEXIT_HLT                           12
#define VMEXIT_EPT_VIOLATION                 48

#define VMEXIT_CONTROLS_LONG_MODE       1 << 9
#define VMEXIT_CONTROLS_LOAD_IA32_EFER  1 << 21
#define VMEXIT_ON_HLT                   1 << 7
#define VMEXIT_ON_PIO                   1 << 24
#define SECONDARY_CONTROLS_ON           1 << 31
#define EPT_ENABLE                      1 << 1
#define UNRESTRICTED_GUEST              1 << 7
#define VMEXIT_ON_DESCRIPTOR            1 << 2

#define VM_ENTRY_64BITGUEST             1 << 9
#define VM_ENTRY_LOADEFER               1 << 15

typedef struct vmcs {
    uintptr_t region;
} vmcs_t;

err_t vmxon();
err_t init_vmcs(vmcs_t* vmcs, initial_guest_state_t* state);

#endif
