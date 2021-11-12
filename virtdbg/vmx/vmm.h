#ifndef __VIRTDBG_VMM_H__
#define __VIRTDBG_VMM_H__
#include <stdbool.h>
#include <util/except.h>
#include <virtdbg.h>

// Vol 3B, APPENDIX H FIELD ENCODING IN VMCS
typedef enum vmcs_field_encoding {
    //! Vol 3C, Table B-1. Encoding for 16-Bit Control Fields (0000_00xx_xxxx_xxx0B)
    VMCS_FIELD_VPID = 0x00000000,
    VMCS_FIELD_POSTED_INTR_NOTIFICATION_VECTOR = 0x00000002,
    VMCS_FIELD_EPTP_INDEX = 0x00000004,

    //! Vol 3C, Table B-2. Encodings for 16-Bit Guest-State Fields (0000_10xx_xxxx_xxx0B)
    VMCS_FIELD_GUEST_ES_SELECTOR = 0x00000800,
    VMCS_FIELD_GUEST_CS_SELECTOR = 0x00000802,
    VMCS_FIELD_GUEST_SS_SELECTOR = 0x00000804,
    VMCS_FIELD_GUEST_DS_SELECTOR = 0x00000806,
    VMCS_FIELD_GUEST_FS_SELECTOR = 0x00000808,
    VMCS_FIELD_GUEST_GS_SELECTOR = 0x0000080a,
    VMCS_FIELD_GUEST_LDTR_SELECTOR = 0x0000080c,
    VMCS_FIELD_GUEST_TR_SELECTOR = 0x0000080e,
    VMCS_FIELD_GUEST_INTR_STATUS = 0x00000810,
    VMCS_FIELD_GUEST_PML_INDEX = 0x00000812,

    //! Vol 3C, Table B-3. Encodings for 16-Bit Host-State Fields (0000_11xx_xxxx_xxx0B)
    VMCS_FIELD_HOST_ES_SELECTOR = 0x00000c00,
    VMCS_FIELD_HOST_CS_SELECTOR = 0x00000c02,
    VMCS_FIELD_HOST_SS_SELECTOR = 0x00000c04,
    VMCS_FIELD_HOST_DS_SELECTOR = 0x00000c06,
    VMCS_FIELD_HOST_FS_SELECTOR = 0x00000c08,
    VMCS_FIELD_HOST_GS_SELECTOR = 0x00000c0a,
    VMCS_FIELD_HOST_TR_SELECTOR = 0x00000c0c,

    //! Vol 3C, Table B-3. Encodings for 16-Bit Host-State Fields (0000_11xx_xxxx_xxx0B)
    VMCS_FIELD_IO_BITMAP_A_FULL = 0x00002000,
    VMCS_FIELD_IO_BITMAP_A_HIGH = 0x00002001,
    VMCS_FIELD_IO_BITMAP_B_FULL = 0x00002002,
    VMCS_FIELD_IO_BITMAP_B_HIGH = 0x00002003,
    VMCS_FIELD_MSR_BITMAP_FULL = 0x00002004,
    VMCS_FIELD_MSR_BITMAP_HIGH = 0x00002005,
    VMCS_FIELD_VM_EXIT_MSR_STORE_ADDR_FULL = 0x00002006,
    VMCS_FIELD_VM_EXIT_MSR_STORE_ADDR_HIGH = 0x00002007,
    VMCS_FIELD_VM_EXIT_MSR_LOAD_ADDR_FULL = 0x00002008,
    VMCS_FIELD_VM_EXIT_MSR_LOAD_ADDR_HIGH = 0x00002009,
    VMCS_FIELD_VM_ENTRY_MSR_LOAD_ADDR_FULL = 0x0000200a,
    VMCS_FIELD_VM_ENTRY_MSR_LOAD_ADDR_HIGH = 0x0000200b,
    VMCS_FIELD_EXECUTIVE_VMCS_PTR_FULL = 0x0000200c,
    VMCS_FIELD_EXECUTIVE_VMCS_PTR_HIGH = 0x0000200d,
    VMCS_FIELD_PML_ADDRESS_FULL = 0x0000200e,
    VMCS_FIELD_PML_ADDRESS_HIGH = 0x0000200f,
    VMCS_FIELD_TSC_OFFSET_FULL = 0x00002010,
    VMCS_FIELD_TSC_OFFSET_HIGH = 0x00002011,
    VMCS_FIELD_VIRTUAL_APIC_PAGE_ADDR_FULL = 0x00002012,
    VMCS_FIELD_VIRTUAL_APIC_PAGE_ADDR_HIGH = 0x00002013,
    VMCS_FIELD_APIC_ACCESS_ADDR_FULL = 0x00002014,
    VMCS_FIELD_APIC_ACCESS_ADDR_HIGH = 0x00002015,
    VMCS_FIELD_PI_DESC_ADDR_FULL = 0x00002016,
    VMCS_FIELD_PI_DESC_ADDR_HIGH = 0x00002017,
    VMCS_FIELD_VM_FUNCTION_CONTROL_FULL = 0x00002018,
    VMCS_FIELD_VM_FUNCTION_CONTROL_HIGH = 0x00002019,
    VMCS_FIELD_EPT_POINTER_FULL = 0x0000201a,
    VMCS_FIELD_EPT_POINTER_HIGH = 0x0000201b,
    VMCS_FIELD_EOI_EXIT_BITMAP0_FULL = 0x0000201c,
    VMCS_FIELD_EOI_EXIT_BITMAP0_HIGH = 0x0000201d,
    VMCS_FIELD_EPTP_LIST_ADDR_FULL = 0x00002024,
    VMCS_FIELD_EPTP_LIST_ADDR_HIGH = 0x00002025,
    VMCS_FIELD_VMREAD_BITMAP_FULL = 0x00002026,
    VMCS_FIELD_VMREAD_BITMAP_HIGH = 0x00002027,
    VMCS_FIELD_VMWRITE_BITMAP_FULL = 0x00002028,
    VMCS_FIELD_VMWRITE_BITMAP_HIGH = 0x00002029,
    VMCS_FIELD_VIRT_EXCEPTION_INFO_FULL = 0x0000202a,
    VMCS_FIELD_VIRT_EXCEPTION_INFO_HIGH = 0x0000202b,
    VMCS_FIELD_XSS_EXIT_BITMAP_FULL = 0x0000202c,
    VMCS_FIELD_XSS_EXIT_BITMAP_HIGH = 0x0000202d,
    VMCS_FIELD_TSC_MULTIPLIER_FULL = 0x00002032,
    VMCS_FIELD_TSC_MULTIPLIER_HIGH = 0x00002033,

    //! Vol 3C, Table B-5. Encodings for 64-Bit Read-Only Data Field (0010_01xx_xxxx_xxxAb)
    VMCS_FIELD_GUEST_PHYSICAL_ADDRESS_FULL = 0x00002400,
    VMCS_FIELD_GUEST_PHYSICAL_ADDRESS_HIGH = 0x00002401,

    //! Vol 3C, Table B-6. Encodings for 64-Bit Guest-State Fields (0010_10xx_xxxx_xxxAb)
    VMCS_FIELD_VMCS_LINK_POINTER_FULL = 0x00002800,
    VMCS_FIELD_VMCS_LINK_POINTER_HIGH = 0x00002801,
    VMCS_FIELD_GUEST_IA32_DEBUGCTL_FULL = 0x00002802,
    VMCS_FIELD_GUEST_IA32_DEBUGCTL_HIGH = 0x00002803,
    VMCS_FIELD_GUEST_PAT_FULL = 0x00002804,
    VMCS_FIELD_GUEST_PAT_HIGH = 0x00002805,
    VMCS_FIELD_GUEST_EFER_FULL = 0x00002806,
    VMCS_FIELD_GUEST_EFER_HIGH = 0x00002807,
    VMCS_FIELD_GUEST_PERF_GLOBAL_CTRL_FULL = 0x00002808,
    VMCS_FIELD_GUEST_PERF_GLOBAL_CTRL_HIGH = 0x00002809,
    VMCS_FIELD_GUEST_PDPTE0_FULL = 0x0000280a,
    VMCS_FIELD_GUEST_PDPTE0_HIGH = 0x0000280b,
    VMCS_FIELD_GUEST_PDPTE1_FULL = 0x0000280c,
    VMCS_FIELD_GUEST_PDPTE1_HIGH = 0x0000280d,
    VMCS_FIELD_GUEST_PDPTE2_FULL = 0x0000280e,
    VMCS_FIELD_GUEST_PDPTE2_HIGH = 0x0000280f,
    VMCS_FIELD_GUEST_PDPTE3_FULL = 0x00002810,
    VMCS_FIELD_GUEST_PDPTE3_HIGH = 0x00002811,
    VMCS_FIELD_GUEST_BNDCFGS_FULL = 0x00002812,
    VMCS_FIELD_GUEST_BNDCFGS_HIGH = 0x00002813,

    //! Vol 3C, Table B-7. Encodings for 64-Bit Host-State Fields (0010_11xx_xxxx_xxxAb)
    VMCS_FIELD_HOST_PAT_FULL = 0x00002c00,
    VMCS_FIELD_HOST_PAT_HIGH = 0x00002c01,
    VMCS_FIELD_HOST_EFER_FULL = 0x00002c02,
    VMCS_FIELD_HOST_EFER_HIGH = 0x00002c03,
    VMCS_FIELD_HOST_PERF_GLOBAL_CTRL_FULL = 0x00002c04,
    VMCS_FIELD_HOST_PERF_GLOBAL_CTRL_HIGH = 0x00002c05,

    //! Vol 3C, Table B-8. Encodings for 32-Bit Control Fields (0100_00xx_xxxx_xxx0B)
    VMCS_FIELD_PINBASED_CTLS = 0x00004000,
    VMCS_FIELD_PROCBASED_CTLS = 0x00004002,
    VMCS_FIELD_EXCEPTION_BITMAP = 0x00004004,
    VMCS_FIELD_PAGE_FAULT_ERROR_CODE_MASK = 0x00004006,
    VMCS_FIELD_PAGE_FAULT_ERROR_CODE_MATCH = 0x00004008,
    VMCS_FIELD_CR3_TARGET_COUNT = 0x0000400a,
    VMCS_FIELD_VMEXIT_CTLS = 0x0000400c,
    VMCS_FIELD_VM_EXIT_MSR_STORE_COUNT = 0x0000400e,
    VMCS_FIELD_VM_EXIT_MSR_LOAD_COUNT = 0x00004010,
    VMCS_FIELD_VMENTRY_CTLS = 0x00004012,
    VMCS_FIELD_VM_ENTRY_MSR_LOAD_COUNT = 0x00004014,
    VMCS_FIELD_VM_ENTRY_INTR_INFO = 0x00004016,
    VMCS_FIELD_VM_ENTRY_EXCEPTION_ERROR_CODE = 0x00004018,
    VMCS_FIELD_VM_ENTRY_INSTRUCTION_LEN = 0x0000401a,
    VMCS_FIELD_TPR_THRESHOLD = 0x0000401c,
    VMCS_FIELD_PROCBASED_CTLS2 = 0x0000401e,
    VMCS_FIELD_PLE_GAP = 0x00004020,
    VMCS_FIELD_PLE_WINDOW = 0x00004022,

    //! Vol 3C, Table B-9. Encodings for 32-Bit Read-Only Data Fields (0100_01xx_xxxx_xxx0B)
    VMCS_FIELD_VM_INSTRUCTION_ERROR = 0x00004400,
    VMCS_FIELD_VM_EXIT_REASON = 0x00004402,
    VMCS_FIELD_VM_EXIT_INTR_INFO = 0x00004404,
    VMCS_FIELD_VM_EXIT_INTR_ERROR_CODE = 0x00004406,
    VMCS_FIELD_IDT_VECTORING_INFO = 0x00004408,
    VMCS_FIELD_IDT_VECTORING_ERROR_CODE = 0x0000440a,
    VMCS_FIELD_VM_EXIT_INSTRUCTION_LEN = 0x0000440c,
    VMCS_FIELD_INSTRUCTION_INFO = 0x0000440e,
    VMCS_FIELD_EPT_VIOLATION_ADDRESS = 0x00002400,

    //! Vol 3C, Table B-10. Encodings for 32-Bit Guest-State Fields (0100_10xx_xxxx_xxx0B)
    VMCS_FIELD_GUEST_ES_LIMIT = 0x00004800,
    VMCS_FIELD_GUEST_CS_LIMIT = 0x00004802,
    VMCS_FIELD_GUEST_SS_LIMIT = 0x00004804,
    VMCS_FIELD_GUEST_DS_LIMIT = 0x00004806,
    VMCS_FIELD_GUEST_FS_LIMIT = 0x00004808,
    VMCS_FIELD_GUEST_GS_LIMIT = 0x0000480a,
    VMCS_FIELD_GUEST_LDTR_LIMIT = 0x0000480c,
    VMCS_FIELD_GUEST_TR_LIMIT = 0x0000480e,
    VMCS_FIELD_GUEST_GDTR_LIMIT = 0x00004810,
    VMCS_FIELD_GUEST_IDTR_LIMIT = 0x00004812,
    VMCS_FIELD_GUEST_ES_AR_BYTES = 0x00004814,
    VMCS_FIELD_GUEST_CS_AR_BYTES = 0x00004816,
    VMCS_FIELD_GUEST_SS_AR_BYTES = 0x00004818,
    VMCS_FIELD_GUEST_DS_AR_BYTES = 0x0000481a,
    VMCS_FIELD_GUEST_FS_AR_BYTES = 0x0000481c,
    VMCS_FIELD_GUEST_GS_AR_BYTES = 0x0000481e,
    VMCS_FIELD_GUEST_LDTR_AR_BYTES = 0x00004820,
    VMCS_FIELD_GUEST_TR_AR_BYTES = 0x00004822,
    VMCS_FIELD_GUEST_INTERRUPTIBILITY_INFO = 0x00004824,
    VMCS_FIELD_GUEST_ACTIVITY_STATE = 0x00004826,
    VMCS_FIELD_GUEST_SMBASE = 0x00004828,
    VMCS_FIELD_GUEST_SYSENTER_CS = 0x0000482a,
    VMCS_FIELD_GUEST_PREEMPTION_TIMER = 0x0000482e,

    //! Vol 3C, Table B-11. Encoding for 32-Bit Host-State Field (0100_11xx_xxxx_xxx0B)
    VMCS_FIELD_HOST_SYSENTER_CS = 0x00004c00,

    //! Vol 3C, Table B-12. Encodings for Natural-Width Control Fields (0110_00xx_xxxx_xxx0B)
    VMCS_FIELD_CR0_GUEST_HOST_MASK = 0x00006000,
    VMCS_FIELD_CR4_GUEST_HOST_MASK = 0x00006002,
    VMCS_FIELD_CR0_READ_SHADOW = 0x00006004,
    VMCS_FIELD_CR4_READ_SHADOW = 0x00006006,
    VMCS_FIELD_CR3_TARGET_VALUE0 = 0x00006008,
    VMCS_FIELD_CR3_TARGET_VALUE1 = 0x0000600a,
    VMCS_FIELD_CR3_TARGET_VALUE2 = 0x0000600c,
    VMCS_FIELD_CR3_TARGET_VALUE3 = 0x0000600e,

    //! Vol 3C, Table B-13. Encodings for Natural-Width Read-Only Data Fields (0110_01xx_xxxx_xxx0B)
    VMCS_FIELD_EXIT_QUALIFICATION = 0x00006400,
    VMCS_FIELD_IO_RCX = 0x00006402,
    VMCS_FIELD_IO_RSI = 0x00006404,
    VMCS_FIELD_IO_RDI = 0x00006406,
    VMCS_FIELD_IO_RIP = 0x00006408,
    VMCS_FIELD_GUEST_LINEAR_ADDRESS = 0x0000640a,

    //! Vol 3C, Table B-14. Encodings for Natural-Width Guest-State Fields (0110_10xx_xxxx_xxx0B)
    VMCS_FIELD_GUEST_CR0 = 0x00006800,
    VMCS_FIELD_GUEST_CR3 = 0x00006802,
    VMCS_FIELD_GUEST_CR4 = 0x00006804,
    VMCS_FIELD_GUEST_ES_BASE = 0x00006806,
    VMCS_FIELD_GUEST_CS_BASE = 0x00006808,
    VMCS_FIELD_GUEST_SS_BASE = 0x0000680a,
    VMCS_FIELD_GUEST_DS_BASE = 0x0000680c,
    VMCS_FIELD_GUEST_FS_BASE = 0x0000680e,
    VMCS_FIELD_GUEST_GS_BASE = 0x00006810,
    VMCS_FIELD_GUEST_LDTR_BASE = 0x00006812,
    VMCS_FIELD_GUEST_TR_BASE = 0x00006814,
    VMCS_FIELD_GUEST_GDTR_BASE = 0x00006816,
    VMCS_FIELD_GUEST_IDTR_BASE = 0x00006818,
    VMCS_FIELD_GUEST_DR7 = 0x0000681a,
    VMCS_FIELD_GUEST_RSP = 0x0000681c,
    VMCS_FIELD_GUEST_RIP = 0x0000681e,
    VMCS_FIELD_GUEST_RFLAGS = 0x00006820,
    VMCS_FIELD_GUEST_PENDING_DBG_EXCEPTIONS = 0x00006822,
    VMCS_FIELD_GUEST_SYSENTER_ESP = 0x00006824,
    VMCS_FIELD_GUEST_SYSENTER_EIP = 0x00006826,

    //! Vol 3C, Table B-15. Encodings for Natural-Width Host-State Fields (0110_11xx_xxxx_xxx0B)
    VMCS_FIELD_HOST_CR0 = 0x00006c00,
    VMCS_FIELD_HOST_CR3 = 0x00006c02,
    VMCS_FIELD_HOST_CR4 = 0x00006c04,
    VMCS_FIELD_HOST_FS_BASE = 0x00006c06,
    VMCS_FIELD_HOST_GS_BASE = 0x00006c08,
    VMCS_FIELD_HOST_TR_BASE = 0x00006c0a,
    VMCS_FIELD_HOST_GDTR_BASE = 0x00006c0c,
    VMCS_FIELD_HOST_IDTR_BASE = 0x00006c0e,
    VMCS_FIELD_HOST_SYSENTER_ESP = 0x00006c10,
    VMCS_FIELD_HOST_SYSENTER_EIP = 0x00006c12,
    VMCS_FIELD_HOST_RSP = 0x00006c14,
    VMCS_FIELD_HOST_RIP = 0x00006c16,
} vmcs_field_encoding_t;

// Vol 3B, Table I-1. Basic Exit Reasons
typedef enum vmexit_reason {
    VMEXIT_REASON_EXCEPTION_NMI = 0,
	VMEXIT_REASON_EXTERNAL_INTERRUPT = 1,
	VMEXIT_REASON_TRIPLE_FAULT = 2,
	VMEXIT_REASON_INIT = 3,
	VMEXIT_REASON_SIPI = 4,
	VMEXIT_REASON_IO_SMI = 5,
	VMEXIT_REASON_OTHER_SMI = 6,
	VMEXIT_REASON_PENDING_VIRT_INTR = 7,
	VMEXIT_REASON_PENDING_VIRT_NMI = 8,
	VMEXIT_REASON_TASK_SWITCH = 9,
	VMEXIT_REASON_CPUID = 10,
	VMEXIT_REASON_GETSEC = 11,
	VMEXIT_REASON_HLT = 12,
	VMEXIT_REASON_INVD = 13,
	VMEXIT_REASON_INVLPG = 14,
	VMEXIT_REASON_RDPMC = 15,
	VMEXIT_REASON_RDTSC = 16,
	VMEXIT_REASON_RSM = 17,
	VMEXIT_REASON_VMCALL = 18,
	VMEXIT_REASON_VMCLEAR = 19,
	VMEXIT_REASON_VMLAUNCH = 20,
	VMEXIT_REASON_VMPTRLD = 21,
	VMEXIT_REASON_VMPTRST = 22,
	VMEXIT_REASON_VMREAD = 23,
	VMEXIT_REASON_VMRESUME = 24,
	VMEXIT_REASON_VMWRITE = 25,
	VMEXIT_REASON_VMXOFF = 26,
	VMEXIT_REASON_VMXON = 27,
	VMEXIT_REASON_CR_ACCESS = 28,
	VMEXIT_REASON_DR_ACCESS = 29,
	VMEXIT_REASON_IO_INSTRUCTION = 30,
	VMEXIT_REASON_MSR_READ = 31,
	VMEXIT_REASON_MSR_WRITE = 32,
	VMEXIT_REASON_INVALID_GUEST_STATE = 33,
	VMEXIT_REASON_MSR_LOADING = 34,
	VMEXIT_REASON_MWAIT_INSTRUCTION = 36,
	VMEXIT_REASON_MONITOR_TRAP_FLAG = 37,
	VMEXIT_REASON_MONITOR_INSTRUCTION = 39,
	VMEXIT_REASON_PAUSE_INSTRUCTION = 40,
	VMEXIT_REASON_MCE_DURING_VMENTRY = 41,
	VMEXIT_REASON_TPR_BELOW_THRESHOLD = 43,
	VMEXIT_REASON_APIC_ACCESS = 44,
	VMEXIT_REASON_ACCESS_GDTR_OR_IDTR = 46,
	VMEXIT_REASON_ACCESS_LDTR_OR_TR = 47,
	VMEXIT_REASON_EPT_VIOLATION = 48,
	VMEXIT_REASON_EPT_MISCONFIG = 49,
	VMEXIT_REASON_INVEPT = 50,
	VMEXIT_REASON_RDTSCP = 51,
	VMEXIT_REASON_VMX_PREEMPTION_TIMER_EXPIRED = 52,
	VMEXIT_REASON_INVVPID = 53,
	VMEXIT_REASON_WBINVD = 54,
	VMEXIT_REASON_XSETBV = 55,
	VMEXIT_REASON_APIC_WRITE = 56,
	VMEXIT_REASON_RDRAND = 57,
	VMEXIT_REASON_INVPCID = 58,
	VMEXIT_REASON_RDSEED = 61,
	VMEXIT_REASON_PML_FULL = 62,
	VMEXIT_REASON_XSAVES = 63,
	VMEXIT_REASON_XRSTORS = 64,
	VMEXIT_REASON_PCOMMIT = 65,
    VMEXIT_REASONS_MAX
} vmexit_reason_t;

//! Vol 3C, Table 24-5. Definitions of Pin-Based VM-Execution Controls
typedef union vmx_pinbased_ctls {
    struct {
        uint32_t external_int_exit : 1;
        uint32_t _reserved0 : 2;
        uint32_t nmi_exit : 1;
        uint32_t _reserved1 : 1;
        uint32_t virt_nmi_exit : 1;
        uint32_t preemption_timer : 1;
        uint32_t process_apic_ints : 1;
        uint32_t _reserved2 : 24;
    };
    uint32_t raw;
} vmx_pinbased_ctls_t;
_Static_assert(sizeof(vmx_pinbased_ctls_t) == sizeof(uint32_t), "invalid size for vmx_pinbased_ctls_t");

//! Vol 3C, Table 24-6. Definitions of Primary Processor-Based VM-Execution Controls
typedef union vmx_procbased_ctls {
    struct {
        uint32_t _reserved0 : 2;
        uint32_t int_window_exit : 1;
        uint32_t use_tsc_offseting : 1;
        uint32_t _reserved1 : 3;
        uint32_t hlt_exit : 1;
        uint32_t _reserved2 : 1;
        uint32_t invlpg_exit : 1;
        uint32_t mwait_exit : 1;
        uint32_t rdpmc_exit : 1;
        uint32_t rdtsc_exit : 1;
        uint32_t _reserved3 : 2;
        uint32_t cr3_load_exit : 1;
        uint32_t cr3_store_exit : 1;
        uint32_t _reserved4 : 2;
        uint32_t cr8_load_exit : 1;
        uint32_t cr8_store_exit : 1;
        uint32_t use_tpr_shadow : 1;
        uint32_t nmi_window_exit : 1;
        uint32_t mov_dr_exit : 1;
        uint32_t uncond_io_exit : 1;
        uint32_t use_io_bitmaps : 1;
        uint32_t _reserved5 : 1;
        uint32_t monitor_trap_flag : 1;
        uint32_t use_msr_bitmaps : 1;
        uint32_t monitor_exit : 1;
        uint32_t pause_exit : 1;
        uint32_t use_procased2 : 1;
    };
    uint32_t raw;
} vmx_procbased_ctls_t;
_Static_assert(sizeof(vmx_procbased_ctls_t) == sizeof(uint32_t), "invalid size for vmx_procbased_ctls_t");

typedef union vmx_procbased_ctls2 {
    struct {
        uint32_t virt_apic_access : 1;
        uint32_t enable_ept : 1;
        uint32_t descriptor_table_exit : 1;
        uint32_t enable_rdtscp : 1;
        uint32_t virt_x2apic_access : 1;
        uint32_t enable_vpid : 1;
        uint32_t wbinvd_exit : 1;
        uint32_t unrestricted_guest : 1;
        uint32_t apic_register : 1;
        uint32_t virt_int_exit : 1;
        uint32_t pause_loop_exit : 1;
        uint32_t rdrand_exit : 1;
        uint32_t invpcid_exit : 1;
        uint32_t enable_vm_func : 1;
        uint32_t enable_shadow_vmcs : 1;
        uint32_t _reserved0 : 1;
        uint32_t rdseed_exit : 1;
        uint32_t enable_pml : 1;
        uint32_t enable_ept_ve : 1;
        uint32_t _reserved1 : 1;
        uint32_t xsave_xrstor_exit : 1;
        uint32_t _reserved2 : 4;
        uint32_t tsc_scaling : 1;
        uint32_t _reserved6 : 6;
    };
    uint32_t raw;
} vmx_procbased_ctls2_t;
_Static_assert(sizeof(vmx_procbased_ctls2_t) == sizeof(uint32_t), "invalid size for vmx_procbased_ctls2_t");

typedef union vmx_exit_ctls {
    struct {
        uint32_t _reserved0 : 2;
        uint32_t save_debug_controls : 1;
        uint32_t _reserved : 6;
        uint32_t is_host_64bit : 1;
        uint32_t _reserved2 : 2;
        uint32_t load_ia32_perf_global_ctrl : 1;
        uint32_t _reserved3 : 2;
        uint32_t ack_int_on_exit : 1;
        uint32_t _reserved4 : 2;
        uint32_t save_ia32_pat : 1;
        uint32_t load_ia32_pat : 1;
        uint32_t save_ia32_efer : 1;
        uint32_t load_ia32_efer : 1;
        uint32_t save_preemption_timer : 1;
        uint32_t _reserved5 : 9;
    };
    uint32_t raw;
} vmx_exit_ctls_t;
_Static_assert(sizeof(vmx_exit_ctls_t) == sizeof(uint32_t), "invalid size for vmx_exit_ctls_t");

typedef union vmx_entry_ctls {
    struct {
        uint32_t _reserved0 : 2;
        uint32_t load_debug_controls : 1;
        uint32_t _reserved1 : 6;
        uint32_t is_guest_64bit : 1;
        uint32_t enter_smm : 1;
        uint32_t disable_dual_monitor : 1;
        uint32_t _reserved2 : 1;
        uint32_t load_ia32_perf_global_ctrl : 1;
        uint32_t load_ia32_pat : 1;
        uint32_t load_ia32_efer : 1;
        uint32_t _reserved3 : 16;
    };
    uint32_t raw;
} vmx_entry_ctls_t;
_Static_assert(sizeof(vmx_entry_ctls_t) == sizeof(uint32_t), "invalid size for vmx_entry_ctls_t");

typedef union vmx_vmexit_reason {
    struct {
        uint32_t exit_reason : 16;
        uint32_t _reserved0 : 12;
        uint32_t pending_mtf : 1;
        uint32_t exit_from_root : 1;
        uint32_t _reserved1 : 1;
        uint32_t entry_failed : 1;
    };
    uint32_t raw;
} vmx_vmexit_reason_t;
_Static_assert(sizeof(vmx_vmexit_reason_t) == sizeof(uint32_t), "invalid size for vmx_vmexit_reason_t");

#define DATA_ACCESS_RIGHT  (0x3 | 1 << 4 | 1 << 7)
#define CODE_ACCESS_RIGHT  (0x3 | 1 << 4 | 1 << 7 | 1 << 13)
#define LDTR_ACCESS_RIGHT  (0x2 | 1 << 7)
#define TR_ACCESS_RIGHT    (11 | 1 << 7)

typedef struct vmcs {
    uintptr_t region;
} vmcs_t;

err_t vmxon();
err_t init_vmcs(vmcs_t* vmcs, initial_guest_state_t* state);

#endif
