#include <vmx/vmm.h>
#include <mm/pmm.h>
#include <arch/intrin.h>
#include <stdint.h>
#include <vmx/ept.h>
#include <stddef.h>
#include <util/except.h>
#include <virtdbg.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/msr.h>

extern void vm_resume(guest_state_t *t);
extern ept_entry_t* g_root_pa;

//enable vmx operation
err_t vmxon() {
    err_t err = NO_ERROR;
    uint32_t* vmxon_region = pallocz_aligned(0x1000, 0x1000);
    TRACE("vmxon region: %x", vmxon_region);

    ia32_feature_control_t control = (ia32_feature_control_t)__rdmsr(MSR_CODE_IA32_FEATURE_CONTROL);
    if(!control.LockBit) {
        //Enabled outside of SMX and lock bit.
        control.LockBit = 1;
        __wrmsr(MSR_CODE_IA32_FEATURE_CONTROL, control.raw);
    }

    if (!control.VmxOutsideSmx) {
        control.VmxOutsideSmx = 1;
        __wrmsr(MSR_CODE_IA32_FEATURE_CONTROL, control.raw);
    }

    //set and unset the correct bits in cr0 and cr4
    uint64_t cr0 = __readcr0().raw;
    cr0 &= __rdmsr(MSR_IA32_VMX_CR0_FIXED1);
    cr0 |= __rdmsr(MSR_IA32_VMX_CR0_FIXED0);
    __writecr0((ia32_cr0_t) { .raw = cr0 });

    ia32_cr4_t cr4 = __readcr4();
    cr4.VMXE = 1;
    cr4.raw &= __rdmsr(MSR_IA32_VMX_CR4_FIXED1);
    cr4.raw |= __rdmsr(MSR_IA32_VMX_CR4_FIXED0);
    __writecr4(cr4);

    //Set vmx revision
    msr_vmx_basic_t vmx_basic = { .raw = __rdmsr(MSR_IA32_VMX_BASIC) };
    *vmxon_region = vmx_basic.vmcs_revision_id;
    uint16_t successful = 0;

    asm volatile("vmxon %[Region]" : "=@ccnc"(successful) : [Region]"m"(vmxon_region) : "memory");

    CHECK_ERROR(successful, ERROR_UNSUPPORTED, "VMX not supported");

cleanup:
    return err;
}

static inline void vmptrld(uintptr_t vmcs) {
    uint8_t ret;
    asm volatile (
        "vmptrld %[pa];"
        "setna %[ret];"
        : [ret]"=rm"(ret)
        : [pa]"m"(vmcs)
        : "cc", "memory");
    ASSERT(!ret, "Error loading VMCS pointer at %X", vmcs);
}

static inline void vmwrite(uint64_t encoding, uint64_t value) {
    uint8_t ret;
    asm volatile (
        "vmwrite %1, %2;"
        "setna %[ret]"
        : [ret]"=rm"(ret)
        : "rm"(value), "r"(encoding)
        : "cc", "memory");
    ASSERT(!ret, "Error writing to %X", encoding);
}

static inline uint64_t vmread(uint64_t encoding) {
    uint64_t tmp;
    uint8_t ret;
    asm volatile(
        "vmread %[encoding], %[value];"
        "setna %[ret];"
        : [value]"=rm"(tmp), [ret]"=rm"(ret)
        : [encoding]"r"(encoding)
        : "cc", "memory");
    ASSERT(!ret, "Error reading from %X", encoding);

    return tmp;
}

static err_t validate_controls(uint32_t ctls, uint64_t msr_ctls) {
    err_t err = NO_ERROR;

    uint32_t msr_ctls_0 = msr_ctls & 0xFFFFFFFF;
    uint32_t msr_ctls_1 = msr_ctls >> 32;

    // validate the bits are set correctly
    CHECK((msr_ctls_0 & ~ctls) == 0, "Got 0 when should be 1 at bits 0b%b", msr_ctls_0 & ~ctls);
    CHECK((~msr_ctls_1 & ctls) == 0, "Got 1 when should be 0 at bits 0b%b", ~(msr_ctls_1 | ~ctls));

cleanup:
    return err;
}

err_t init_vmcs(vmcs_t* vmcs, initial_guest_state_t* state) {
    err_t err = NO_ERROR;
    msr_vmx_basic_t vmx_basic = { .raw = __rdmsr(MSR_IA32_VMX_BASIC) };

    // Allocate a vmcs region
    vmcs->region = (uintptr_t)pallocz_aligned(vmx_basic.vmcs_size, 0x1000);

    // set the vmx revision
    *(uint32_t*)(vmcs->region) = vmx_basic.vmcs_revision_id;

    // load the vmcs
    TRACE("VMCS: 0x%p, %d bytes", vmcs->region, vmx_basic.vmcs_size);
    TRACE("\tRevision: %04x", vmx_basic.vmcs_revision_id);
    vmptrld(vmcs->region);

    //
    // Setup the pin based controls, these are
    // mostly interrupt controls
    //
    {
        uint64_t allowed_pinbased_ctls = __rdmsr(MSR_IA32_VMX_PINBASED_CTLS);
        vmx_pinbased_ctls_t pinbased_ctls = { .raw = (allowed_pinbased_ctls & 0xFFFFFFFF) & (allowed_pinbased_ctls >> 32) };
        TRACE("default pinbased_ctls:");
        if (pinbased_ctls.external_int_exit) TRACE("\texternal_int_exit");
        if (pinbased_ctls.nmi_exit) TRACE("\tnmi_exit");
        if (pinbased_ctls.virt_nmi_exit) TRACE("\tvirt_nmi_exit");
        if (pinbased_ctls.preemption_timer) TRACE("\tpreemption_timer");
        if (pinbased_ctls.process_apic_ints) TRACE("\tprocess_apic_ints");
        CHECK_AND_RETHROW(validate_controls(pinbased_ctls.raw, allowed_pinbased_ctls));
        vmwrite(VMCS_FIELD_PINBASED_CTLS, pinbased_ctls.raw);
    }

    //
    // Setup the proc based controls, these are mostly exit reasons
    // do to certain instructions being executed and other configurations
    // of vmx (like enabling ept)
    //
    {
        uint64_t allowed_procbased_ctls = __rdmsr(MSR_IA32_VMX_PROCBASED_CTLS);
        vmx_procbased_ctls_t procbased_ctls = { .raw = (allowed_procbased_ctls & 0xFFFFFFFF) & (allowed_procbased_ctls >> 32) };
        TRACE("default procbased_ctls:");
        if (procbased_ctls.int_window_exit) TRACE("\tint_window_exit");
        if (procbased_ctls.use_tsc_offseting) TRACE("\tuse_tsc_offseting");
        if (procbased_ctls.hlt_exit) TRACE("\thlt_exit");
        if (procbased_ctls.invlpg_exit) TRACE("\tinvlpg_exit");
        if (procbased_ctls.mwait_exit) TRACE("\tmwait_exit");
        if (procbased_ctls.rdpmc_exit) TRACE("\trdpmc_exit");
        if (procbased_ctls.rdtsc_exit) TRACE("\trdtsc_exit");
        if (procbased_ctls.cr3_load_exit) TRACE("\tcr3_load_exit");
        if (procbased_ctls.cr3_store_exit) TRACE("\tcr3_store_exit");
        if (procbased_ctls.cr8_load_exit) TRACE("\tcr8_load_exit");
        if (procbased_ctls.cr8_store_exit) TRACE("\tcr8_store_exit");
        if (procbased_ctls.use_tpr_shadow) TRACE("\tuse_tpr_shadow");
        if (procbased_ctls.nmi_window_exit) TRACE("\tnmi_window_exit");
        if (procbased_ctls.mov_dr_exit) TRACE("\tmov_dr_exit");
        if (procbased_ctls.uncond_io_exit) TRACE("\tuncond_io_exit");
        if (procbased_ctls.use_io_bitmaps) TRACE("\tuse_io_bitmaps");
        if (procbased_ctls.monitor_trap_flag) TRACE("\tmonitor_trap_flag");
        if (procbased_ctls.use_msr_bitmaps) TRACE("\tuse_msr_bitmaps");
        if (procbased_ctls.monitor_exit) TRACE("\tmonitor_exit");
        if (procbased_ctls.pause_exit) TRACE("\tpause_exit");
        if (procbased_ctls.use_procased2) TRACE("\tuse_procased2");
        procbased_ctls.use_procased2 = 1;
        CHECK_AND_RETHROW(validate_controls(procbased_ctls.raw, allowed_procbased_ctls));
        vmwrite(VMCS_FIELD_PROCBASED_CTLS, procbased_ctls.raw);
    }

    //
    // from this we only really need to enable EPT so we can map stuff on demand (saves space)
    // and so we can have unrestricted guest, so the kernel can do whatever it wants
    //
    {
        uint64_t allowed_procbased_ctls2 = __rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
        vmx_procbased_ctls2_t procbased_ctls2 = { .raw = (allowed_procbased_ctls2 & 0xFFFFFFFF) & (allowed_procbased_ctls2 >> 32) };
        TRACE("default procbased_ctls2:");
        if (procbased_ctls2.virt_apic_access) TRACE("\tvirt_apic_access");
        if (procbased_ctls2.enable_ept) TRACE("\tenable_ept");
        if (procbased_ctls2.descriptor_table_exit) TRACE("\tdescriptor_table_exit");
        if (procbased_ctls2.enable_rdtscp) TRACE("\tenable_rdtscp");
        if (procbased_ctls2.virt_x2apic_access) TRACE("\tvirt_x2apic_access");
        if (procbased_ctls2.enable_vpid) TRACE("\tenable_vpid");
        if (procbased_ctls2.wbinvd_exit) TRACE("\twbinvd_exit");
        if (procbased_ctls2.unrestricted_guest) TRACE("\tunrestricted_guest");
        if (procbased_ctls2.apic_register) TRACE("\tapic_register");
        if (procbased_ctls2.virt_int_exit) TRACE("\tvirt_int_exit");
        if (procbased_ctls2.pause_loop_exit) TRACE("\tpause_loop_exit");
        if (procbased_ctls2.rdrand_exit) TRACE("\trdrand_exit");
        if (procbased_ctls2.invpcid_exit) TRACE("\tinvpcid_exit");
        if (procbased_ctls2.enable_vm_func) TRACE("\tenable_vm_func");
        if (procbased_ctls2.enable_shadow_vmcs) TRACE("\tenable_shadow_vmcs");
        if (procbased_ctls2.rdseed_exit) TRACE("\trdseed_exit");
        if (procbased_ctls2.enable_pml) TRACE("\tenable_pml");
        if (procbased_ctls2.enable_ept_ve) TRACE("\tenable_ept_ve");
        if (procbased_ctls2.xsave_xrstor_exit) TRACE("\txsave_xrstor_exit");
        if (procbased_ctls2.tsc_scaling) TRACE("\ttsc_scaling");
        procbased_ctls2.enable_ept = 1;
        procbased_ctls2.unrestricted_guest = 1;
        CHECK_AND_RETHROW(validate_controls(procbased_ctls2.raw, allowed_procbased_ctls2));
        vmwrite(VMCS_FIELD_PROCBASED_CTLS2, procbased_ctls2.raw);
    }

    //
    // we don't want to exit on any exception
    //
    vmwrite(VMCS_FIELD_EXCEPTION_BITMAP, 0);

    //
    // Setup the vmexit controls, just tell it we are a 64bit host
    //
    {
        uint64_t allowed_exit_ctls = __rdmsr(MSR_IA32_VMX_EXIT_CTLS);
        vmx_exit_ctls_t exit_ctls = { .raw = (allowed_exit_ctls & 0xFFFFFFFF) & (allowed_exit_ctls >> 32) };
        TRACE("default exit_ctls:");
        if (exit_ctls.save_debug_controls) TRACE("\tsave_debug_controls");
        if (exit_ctls.is_host_64bit) TRACE("\tis_host_64bit");
        if (exit_ctls.load_ia32_perf_global_ctrl) TRACE("\tload_ia32_perf_global_ctrl");
        if (exit_ctls.ack_int_on_exit) TRACE("\tack_int_on_exit");
        if (exit_ctls.save_ia32_pat) TRACE("\tsave_ia32_pat");
        if (exit_ctls.load_ia32_pat) TRACE("\tload_ia32_pat");
        if (exit_ctls.save_ia32_efer) TRACE("\tsave_ia32_efer");
        if (exit_ctls.load_ia32_efer) TRACE("\tload_ia32_efer");
        if (exit_ctls.save_preemption_timer) TRACE("\tsave_preemption_timer");
        exit_ctls.is_host_64bit = 1;
        exit_ctls.load_ia32_efer = 1;
        CHECK_AND_RETHROW(validate_controls(exit_ctls.raw, allowed_exit_ctls));
        vmwrite(VMCS_FIELD_VMEXIT_CTLS, exit_ctls.raw);
    }

    //
    // Setup the vmx entry controls, tell it we are
    //
    {
        msr_efer_t guest_efer = { .raw = state->efer };
        uint64_t allowed_entry_ctls = __rdmsr(MSR_IA32_VMX_ENTRY_CTLS);
        vmx_entry_ctls_t entry_ctls = { .raw = (allowed_entry_ctls & 0xFFFFFFFF) & (allowed_entry_ctls >> 32) };
        TRACE("default entry_ctls:");
        if (entry_ctls.load_debug_controls) TRACE("\tload_debug_controls");
        if (entry_ctls.is_guest_64bit) TRACE("\tis_guest_64bit");
        if (entry_ctls.enter_smm) TRACE("\tenter_smm");
        if (entry_ctls.disable_dual_monitor) TRACE("\tdisable_dual_monitor");
        if (entry_ctls.load_ia32_perf_global_ctrl) TRACE("\tload_ia32_perf_global_ctrl");
        if (entry_ctls.load_ia32_pat) TRACE("\tload_ia32_pat");
        if (entry_ctls.load_ia32_efer) TRACE("\tload_ia32_efer");
        entry_ctls.is_guest_64bit = guest_efer.long_mode_active;
        entry_ctls.load_ia32_efer = 1;
        CHECK_AND_RETHROW(validate_controls(entry_ctls.raw, allowed_entry_ctls));
        vmwrite(VMCS_FIELD_VMENTRY_CTLS, entry_ctls.raw);
    }

    //
    // Setup the EPT
    //
    vmwrite(VMCS_FIELD_EPT_POINTER_FULL, (uintptr_t)g_root_pa | EPT_WB | EPT_PAGEWALK(EPT_LEVELS));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Set the guest state
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Setup the guest state based on the initial values
    // TODO: we need to setup a ton of state in here
    vmwrite(VMCS_FIELD_GUEST_ES_SELECTOR, state->es);
    vmwrite(VMCS_FIELD_GUEST_CS_SELECTOR, state->cs);
    vmwrite(VMCS_FIELD_GUEST_DS_SELECTOR, state->ds);
    vmwrite(VMCS_FIELD_GUEST_FS_SELECTOR, state->fs);
    vmwrite(VMCS_FIELD_GUEST_GS_SELECTOR, state->gs);
    vmwrite(VMCS_FIELD_GUEST_SS_SELECTOR, state->ss);
    vmwrite(VMCS_FIELD_GUEST_TR_SELECTOR, 0x0);
    vmwrite(VMCS_FIELD_GUEST_LDTR_SELECTOR, 0x0);
    vmwrite(VMCS_FIELD_GUEST_CS_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_DS_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_ES_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_FS_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_GS_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_SS_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_LDTR_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_IDTR_BASE, (size_t)state->idt.address);
    vmwrite(VMCS_FIELD_GUEST_GDTR_BASE, (size_t)state->gdt.address);
    vmwrite(VMCS_FIELD_GUEST_TR_BASE, 0x0);
    vmwrite(VMCS_FIELD_GUEST_CS_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_DS_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_ES_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_FS_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_GS_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_SS_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_LDTR_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_TR_LIMIT, 0xffff);
    vmwrite(VMCS_FIELD_GUEST_GDTR_LIMIT, state->gdt.size);
    vmwrite(VMCS_FIELD_GUEST_IDTR_LIMIT, state->idt.size);

    vmwrite(VMCS_FIELD_HOST_ES_SELECTOR, GDT_DATA);
    vmwrite(VMCS_FIELD_HOST_CS_SELECTOR, GDT_CODE);
    vmwrite(VMCS_FIELD_HOST_SS_SELECTOR, GDT_DATA);
    vmwrite(VMCS_FIELD_HOST_DS_SELECTOR, GDT_DATA);
    vmwrite(VMCS_FIELD_HOST_FS_SELECTOR, GDT_DATA);
    vmwrite(VMCS_FIELD_HOST_GS_SELECTOR, GDT_DATA);

    vmwrite(VMCS_FIELD_GUEST_CS_AR_BYTES, CODE_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_DS_AR_BYTES, DATA_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_ES_AR_BYTES, DATA_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_FS_AR_BYTES, DATA_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_GS_AR_BYTES, DATA_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_SS_AR_BYTES, DATA_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_LDTR_AR_BYTES, LDTR_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_TR_AR_BYTES, TR_ACCESS_RIGHT);
    vmwrite(VMCS_FIELD_GUEST_DR7, state->dr7);
    vmwrite(VMCS_FIELD_GUEST_RFLAGS, state->rflags);
    vmwrite(VMCS_FIELD_GUEST_EFER_FULL, state->efer);

    vmwrite(VMCS_FIELD_GUEST_INTR_STATUS, 0);
    vmwrite(VMCS_FIELD_GUEST_PML_INDEX, 0);

    // copy fixed values to guest
    uint64_t cr0FixedGuest = __rdmsr(MSR_IA32_VMX_CR0_FIXED0);
    uint32_t cr0FixedLo = (uint32_t)cr0FixedGuest;
    uint32_t cr0FixedHi = (uint32_t)(cr0FixedGuest >> 32);
    vmwrite(VMCS_FIELD_GUEST_CR0, cr0FixedLo | ((uint64_t)cr0FixedHi) << 32 | state->cr0);
    vmwrite(VMCS_FIELD_GUEST_CR3, state->cr3);
    uint64_t cr4FixedGuest = __rdmsr(MSR_IA32_VMX_CR4_FIXED0);
    uint32_t cr4FixedLo = (uint32_t)cr4FixedGuest;
    uint32_t cr4FixedHi = (uint32_t)(cr4FixedGuest >> 32);
    vmwrite(VMCS_FIELD_GUEST_CR4, cr4FixedLo | ((uint64_t)cr4FixedHi) << 32 | state->cr4);
    vmwrite(VMCS_FIELD_GUEST_INTERRUPTIBILITY_INFO, 0x0);
    vmwrite(VMCS_FIELD_GUEST_ACTIVITY_STATE, 0x0);

    vmwrite(VMCS_FIELD_GUEST_RSP, state->rsp);
    vmwrite(VMCS_FIELD_GUEST_RIP, state->rip);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Setup everything needed for nested virtualization
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // TODO: this

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Set the host state
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // set the cr0 according to fixed values and values we want
    ia32_cr0_t cr0_fixed = { .raw = __rdmsr(MSR_IA32_VMX_CR0_FIXED0) };
    cr0_fixed.PE = 1;
    cr0_fixed.PG = 1;
    vmwrite(VMCS_FIELD_HOST_CR0, cr0_fixed.raw);

    // set the cr4 according to fixed values and values we want
    ia32_cr4_t cr4_fixed = { .raw = __rdmsr(MSR_IA32_VMX_CR4_FIXED0) };
    cr4_fixed.PAE = 1;
    cr4_fixed.PSE = 1;
    cr4_fixed.VMXE = 1;
    vmwrite(VMCS_FIELD_HOST_CR4, cr4_fixed.raw);

    // We must have a valid Task selector for VMX to work
    vmwrite(VMCS_FIELD_HOST_TR_SELECTOR, 0x18);

    // Set the CR3 as the page table we have
    vmwrite(VMCS_FIELD_HOST_CR3, __readcr3());

    // Set the gdt and idt of the host
    vmwrite(VMCS_FIELD_HOST_GDTR_BASE, (size_t)g_gdt.address);
    vmwrite(VMCS_FIELD_HOST_IDTR_BASE, (size_t)g_idt.address);

    // setup a proper efer for the vm
    msr_efer_t efer = {
        .long_mode_active = 1,
        .long_mode_enable = 1,
    };
    vmwrite(VMCS_FIELD_HOST_EFER_FULL, efer.raw);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // launch the VM, execution will resume at exit_handler
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    extern void vmlaunch_first(guest_state_t* t);
    vmlaunch_first(&state->gprstate);

cleanup:
    return err;
}

static const char* m_vmexit_strings[VMEXIT_REASONS_MAX] = {
    [VMEXIT_REASON_EXCEPTION_NMI] = "VMEXIT_REASON_EXCEPTION_NMI",
	[VMEXIT_REASON_EXTERNAL_INTERRUPT] = "VMEXIT_REASON_EXTERNAL_INTERRUPT",
	[VMEXIT_REASON_TRIPLE_FAULT] = "VMEXIT_REASON_TRIPLE_FAULT",
	[VMEXIT_REASON_INIT] = "VMEXIT_REASON_INIT",
	[VMEXIT_REASON_SIPI] = "VMEXIT_REASON_SIPI",
	[VMEXIT_REASON_IO_SMI] = "VMEXIT_REASON_IO_SMI",
	[VMEXIT_REASON_OTHER_SMI] = "VMEXIT_REASON_OTHER_SMI",
	[VMEXIT_REASON_PENDING_VIRT_INTR] = "VMEXIT_REASON_PENDING_VIRT_INTR",
	[VMEXIT_REASON_PENDING_VIRT_NMI] = "VMEXIT_REASON_PENDING_VIRT_NMI",
	[VMEXIT_REASON_TASK_SWITCH] = "VMEXIT_REASON_TASK_SWITCH",
	[VMEXIT_REASON_CPUID] = "VMEXIT_REASON_CPUID",
	[VMEXIT_REASON_GETSEC] = "VMEXIT_REASON_GETSEC",
	[VMEXIT_REASON_HLT] = "VMEXIT_REASON_HLT",
	[VMEXIT_REASON_INVD] = "VMEXIT_REASON_INVD",
	[VMEXIT_REASON_INVLPG] = "VMEXIT_REASON_INVLPG",
	[VMEXIT_REASON_RDPMC] = "VMEXIT_REASON_RDPMC",
	[VMEXIT_REASON_RDTSC] = "VMEXIT_REASON_RDTSC",
	[VMEXIT_REASON_RSM] = "VMEXIT_REASON_RSM",
	[VMEXIT_REASON_VMCALL] = "VMEXIT_REASON_VMCALL",
	[VMEXIT_REASON_VMCLEAR] = "VMEXIT_REASON_VMCLEAR",
	[VMEXIT_REASON_VMLAUNCH] = "VMEXIT_REASON_VMLAUNCH",
	[VMEXIT_REASON_VMPTRLD] = "VMEXIT_REASON_VMPTRLD",
	[VMEXIT_REASON_VMPTRST] = "VMEXIT_REASON_VMPTRST",
	[VMEXIT_REASON_VMREAD] = "VMEXIT_REASON_VMREAD",
	[VMEXIT_REASON_VMRESUME] = "VMEXIT_REASON_VMRESUME",
	[VMEXIT_REASON_VMWRITE] = "VMEXIT_REASON_VMWRITE",
	[VMEXIT_REASON_VMXOFF] = "VMEXIT_REASON_VMXOFF",
	[VMEXIT_REASON_VMXON] = "VMEXIT_REASON_VMXON",
	[VMEXIT_REASON_CR_ACCESS] = "VMEXIT_REASON_CR_ACCESS",
	[VMEXIT_REASON_DR_ACCESS] = "VMEXIT_REASON_DR_ACCESS",
	[VMEXIT_REASON_IO_INSTRUCTION] = "VMEXIT_REASON_IO_INSTRUCTION",
	[VMEXIT_REASON_MSR_READ] = "VMEXIT_REASON_MSR_READ",
	[VMEXIT_REASON_MSR_WRITE] = "VMEXIT_REASON_MSR_WRITE",
	[VMEXIT_REASON_INVALID_GUEST_STATE] = "VMEXIT_REASON_INVALID_GUEST_STATE",
	[VMEXIT_REASON_MSR_LOADING] = "VMEXIT_REASON_MSR_LOADING",
	[VMEXIT_REASON_MWAIT_INSTRUCTION] = "VMEXIT_REASON_MWAIT_INSTRUCTION",
	[VMEXIT_REASON_MONITOR_TRAP_FLAG] = "VMEXIT_REASON_MONITOR_TRAP_FLAG",
	[VMEXIT_REASON_MONITOR_INSTRUCTION] = "VMEXIT_REASON_MONITOR_INSTRUCTION",
	[VMEXIT_REASON_PAUSE_INSTRUCTION] = "VMEXIT_REASON_PAUSE_INSTRUCTION",
	[VMEXIT_REASON_MCE_DURING_VMENTRY] = "VMEXIT_REASON_MCE_DURING_VMENTRY",
	[VMEXIT_REASON_TPR_BELOW_THRESHOLD] = "VMEXIT_REASON_TPR_BELOW_THRESHOLD",
	[VMEXIT_REASON_APIC_ACCESS] = "VMEXIT_REASON_APIC_ACCESS",
	[VMEXIT_REASON_ACCESS_GDTR_OR_IDTR] = "VMEXIT_REASON_ACCESS_GDTR_OR_IDTR",
	[VMEXIT_REASON_ACCESS_LDTR_OR_TR] = "VMEXIT_REASON_ACCESS_LDTR_OR_TR",
	[VMEXIT_REASON_EPT_VIOLATION] = "VMEXIT_REASON_EPT_VIOLATION",
	[VMEXIT_REASON_EPT_MISCONFIG] = "VMEXIT_REASON_EPT_MISCONFIG",
	[VMEXIT_REASON_INVEPT] = "VMEXIT_REASON_INVEPT",
	[VMEXIT_REASON_RDTSCP] = "VMEXIT_REASON_RDTSCP",
	[VMEXIT_REASON_VMX_PREEMPTION_TIMER_EXPIRED] = "VMEXIT_REASON_VMX_PREEMPTION_TIMER_EXPIRED",
	[VMEXIT_REASON_INVVPID] = "VMEXIT_REASON_INVVPID",
	[VMEXIT_REASON_WBINVD] = "VMEXIT_REASON_WBINVD",
	[VMEXIT_REASON_XSETBV] = "VMEXIT_REASON_XSETBV",
	[VMEXIT_REASON_APIC_WRITE] = "VMEXIT_REASON_APIC_WRITE",
	[VMEXIT_REASON_RDRAND] = "VMEXIT_REASON_RDRAND",
	[VMEXIT_REASON_INVPCID] = "VMEXIT_REASON_INVPCID",
	[VMEXIT_REASON_RDSEED] = "VMEXIT_REASON_RDSEED",
	[VMEXIT_REASON_PML_FULL] = "VMEXIT_REASON_PML_FULL",
	[VMEXIT_REASON_XSAVES] = "VMEXIT_REASON_XSAVES",
	[VMEXIT_REASON_XRSTORS] = "VMEXIT_REASON_XRSTORS",
	[VMEXIT_REASON_PCOMMIT] = "VMEXIT_REASON_PCOMMIT",
};

void exit_handler(guest_state_t *s) {
    while (1) {
        size_t error = vmread(VMCS_FIELD_VM_INSTRUCTION_ERROR);
        if (error) {
            TRACE("VM entry error: %x", error);
        }

        vmx_vmexit_reason_t reason = { .raw = vmread(VMCS_FIELD_VM_EXIT_REASON) };

        uint16_t exit_reason = reason.exit_reason;
        switch (exit_reason) {
            case VMEXIT_REASON_EPT_VIOLATION: {
                size_t address = vmread(0x00002400);
                ept_map(address & ~(0x1000-1));
            } break;

            case VMEXIT_REASON_HLT: {
                TRACE("Guest invoked HLT, halting...");
                asm("hlt");
            } break;

            case VMEXIT_REASON_EXCEPTION_NMI: {
                TRACE("Guest got NMI, ignoring");
            } break;

            default: {
                if (exit_reason < VMEXIT_REASONS_MAX) {
                    ASSERT(0, "Unhandled vmexit: %s (0x%04x)", m_vmexit_strings[exit_reason], reason);
                } else {
                    ASSERT(0, "unknown vmexit: 0x%04x", reason.exit_reason);
                }
            } break;
        }

        ASSERT(!reason.entry_failed, "VMX Entry Failed");

        vm_resume(s);

        //VMX sets out gdt and idt limits to all 1s, so fix that
        __lgdt(g_gdt);
        __lidt(g_idt);
    }
}
