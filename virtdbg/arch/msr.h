#ifndef __VIRTDBG_MSR_H__
#define __VIRTDBG_MSR_H__

#define MSR_IA32_EFER                            0xC0000080
typedef union msr_efer {
    struct {
        uint32_t syscall_enable : 1;
        uint32_t _reserved1:7;
        uint32_t long_mode_enable : 1;
        uint32_t _reserved2:1;
        uint32_t long_mode_active : 1;
        uint32_t execute_disable_bit_enable : 1;
        uint32_t _reserved3:20;
        uint32_t _reserved4:32;
    };
    uint64_t raw;
} msr_efer_t;

#define MSR_IA32_VMX_BASIC                       0x00000480
typedef union msr_vmx_basic {
    struct {
        uint32_t vmcs_revision_id : 31;
        uint32_t _must_be_zero : 1;
        uint32_t vmcs_size : 13;
        uint32_t _reserved0 : 3;
        uint32_t vmcs_address_width : 1;
        uint32_t dual_monitor : 1;
        uint32_t memory_type : 4;
#define MSR_VMX_BASIC_MEMORY_TYPE_UNCACHEABLE  0x00
#define MSR_VMX_BASIC_MEMORY_TYPE_WRITE_BACK   0x06
        uint32_t ins_out_reporting : 1;
        uint32_t vmx_controls : 1;
        uint32_t _reserved1 : 8;
    };
    uint64_t raw;
} msr_vmx_basic_t;

#define MSR_IA32_VMX_PINBASED_CTLS               0x00000481
#define MSR_IA32_VMX_PROCBASED_CTLS              0x00000482
#define MSR_IA32_VMX_EXIT_CTLS                   0x00000483
#define MSR_IA32_VMX_ENTRY_CTLS                  0x00000484

#define MSR_IA32_VMX_MISC                        0x00000485
typedef union msr_vmx_misc {
    struct {
        uint32_t vmtimer_ratio : 5;
        uint32_t vmexit_efer_lma : 1;
        uint32_t hlt_activity_state_supported : 1;
        uint32_t shutdown_activity_state_supported : 1;
        uint32_t wait_for_sipi_activity_State_supported : 1;
        uint32_t _reserved0 : 5;
        uint32_t processor_trace_supported : 1;
        uint32_t smbase_msr_supported : 1;
        uint32_t number_of_cr3_target_values : 9;
        uint32_t msr_store_list_maximum : 3;
        uint32_t block_smi_supported : 1;
        uint32_t vmwrite_supported : 1;
        uint32_t vminject_supported : 1;
        uint32_t _reserved2 : 1;
        uint32_t mseg_revision_identifier : 32;
    };
    uint64_t raw;
} msr_vmx_misc_t;

#define MSR_IA32_VMX_CR0_FIXED0                  0x00000486
#define MSR_IA32_VMX_CR0_FIXED1                  0x00000487
#define MSR_IA32_VMX_CR4_FIXED0                  0x00000488
#define MSR_IA32_VMX_CR4_FIXED1                  0x00000489
#define MSR_IA32_VMX_VMCS_ENUM                   0x0000048A
#define MSR_IA32_VMX_PROCBASED_CTLS2             0x0000048B
#define MSR_IA32_VMX_EPT_VPID_CAP                0x0000048C
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS          0x0000048D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS         0x0000048E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS              0x0000048F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS             0x00000490
#define MSR_IA32_VMX_VMFUNC                      0x00000491


#endif //__VIRTDBG_MSR_H__
