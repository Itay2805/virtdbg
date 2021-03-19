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

extern void vm_resume(guest_state_t *t);
extern ept_entry_t* g_root_pa;

//enable vmx operation
err_t vmxon() {
    err_t err = NO_ERROR;
    uint32_t* vmxon_region = pallocz_aligned(0x1000, 0x1000);

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
    cr0 &= __rdmsr(IA32_VMX_CR0_FIXED1_MSR);
    cr0 |= __rdmsr(IA32_VMX_CR0_FIXED0_MSR);
    __writecr0((ia32_cr0_t) { .raw = cr0 });

    ia32_cr4_t cr4 = __readcr4();
    cr4.VMXE = 1;
    cr4.raw &= __rdmsr(IA32_VMX_CR4_FIXED1_MSR);
    cr4.raw |= __rdmsr(IA32_VMX_CR4_FIXED0_MSR);
    __writecr4(cr4);

    //Set vmx revision
    uint32_t vmx_revision = __rdmsr(IA32_VMX_BASIC_MSR);
    *vmxon_region = vmx_revision;
    uint16_t successful = 0;
    asm volatile(
        "vmxon %1;"
        "movq $0, %%rdx;"
        "success:"
        "movq $1, %%rdx;"
        :"=d"(successful)
        :"m"(vmxon_region)
        :"memory", "cc"
    );

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

err_t init_vmcs(vmcs_t* vmcs, initial_guest_state_t* state) {
    err_t err = NO_ERROR;

    vmcs->region = (uintptr_t)pallocz_aligned(0x1000, 0x1000);

    //set the vmx revision for some reason
    uint32_t vmx_revision = __rdmsr(IA32_VMX_BASIC_MSR);
    *(uint32_t*)(vmcs->region) = vmx_revision;

    vmptrld(vmcs->region);

    //determine which pin-based/proc-based/exit controls have to be enabled
    uint64_t allowedPinBased = __rdmsr(MSR_IA32_VMX_PINBASED_CTLS);
    uint32_t pinbased = (uint32_t)allowedPinBased & (uint32_t)(allowedPinBased >> 32);
    vmwrite(PIN_BASED_VM_EXEC_CONTROLS, pinbased | 1);

    uint64_t allowedCpu = __rdmsr(MSR_IA32_VMX_PROCBASED_CTLS);
    uint32_t cpu = (uint32_t)allowedCpu & (uint32_t)(allowedCpu >> 32);
    uint64_t allowedCpu2 = __rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2) >> 32;

    ASSERT((allowedCpu & SECONDARY_CONTROLS_ON), "VMX secondary controls not supported");
    ASSERT((allowedCpu2 & EPT_ENABLE), "EPT not supported");
    ASSERT((allowedCpu2 & UNRESTRICTED_GUEST), "Unrestricted guest not supported");

    vmwrite(PROC_BASED_VM_EXEC_CONTROLS,
            cpu |
            VMEXIT_ON_HLT |
            SECONDARY_CONTROLS_ON);
    vmwrite(PROC_BASED_VM_EXEC_CONTROLS2,
            EPT_ENABLE |
            UNRESTRICTED_GUEST);
    vmwrite(EXCEPTION_BITMAP, 0);

    uint64_t vmExitCtrls = __rdmsr(IA32_VMX_VM_EXIT_CTLS_MSR);
    uint32_t vmExitCtrlsLo = (uint32_t)vmExitCtrls;
    uint32_t vmExitCtrlsHi = (uint32_t)(vmExitCtrls >> 32);
    uint32_t vm_exit_ctls = 0;
    vm_exit_ctls |= VMEXIT_CONTROLS_LONG_MODE;
    vm_exit_ctls |= VMEXIT_CONTROLS_LOAD_IA32_EFER; // Load IA32_EFER on vm-exit
    vm_exit_ctls |= vmExitCtrlsLo;
    vm_exit_ctls &= vmExitCtrlsHi;
    vmwrite(VM_EXIT_CONTROLS, vm_exit_ctls);

    uint64_t vmEntryCtrls = __rdmsr(IA32_VMX_VM_ENTRY_CTLS_MSR);
    uint32_t vmEntryCtrlsLo = (uint32_t)vmEntryCtrls;
    uint32_t vmEntryCtrlsHi = (uint32_t)(vmEntryCtrls >> 32);
    uint32_t vm_entry_ctls = 0;
    vm_entry_ctls |= vmEntryCtrlsLo;
    vm_entry_ctls &= vmEntryCtrlsHi;
    vm_entry_ctls |= VM_ENTRY_LOADEFER;
    vm_entry_ctls |= VM_ENTRY_64BITGUEST;
    vmwrite(VM_ENTRY_CONTROLS, vm_entry_ctls);

    uint64_t cr0 = __readcr0().raw;
    uint64_t cr4 = __readcr4().raw;

    //start setting up host state for the next vmexit
    //check which bits of cr0/cr4 have to be set
    uint32_t cr0Fixed = (uint32_t)__rdmsr(IA32_VMX_CR0_FIXED0_MSR);
    vmwrite(HOST_CR0, cr0Fixed | cr0);
    uint32_t cr4Fixed = (uint32_t)__rdmsr(IA32_VMX_CR4_FIXED0_MSR);
    vmwrite(HOST_CR4, cr4Fixed | cr4);

    vmwrite(HOST_GDTR_BASE, (size_t)g_gdt.address);
    vmwrite(HOST_IDTR_BASE, (size_t)g_idt.address);
    //save efer
    vmwrite(HOST_EFER_FULL, __rdmsr(EFER));

    //Set up guest state on vm entry.
    //same values as gdt.c
    vmwrite(GUEST_ES_SELECTOR, state->es);
    vmwrite(GUEST_CS_SELECTOR, state->cs);
    vmwrite(GUEST_DS_SELECTOR, state->ds);
    vmwrite(GUEST_FS_SELECTOR, state->fs);
    vmwrite(GUEST_GS_SELECTOR, state->gs);
    vmwrite(GUEST_SS_SELECTOR, state->ss);
    vmwrite(GUEST_TR_SELECTOR, 0x0);
    vmwrite(GUEST_LDTR_SELECTOR, 0x0);
    vmwrite(GUEST_CS_BASE, 0x0);
    vmwrite(GUEST_DS_BASE, 0x0);
    vmwrite(GUEST_ES_BASE, 0x0);
    vmwrite(GUEST_FS_BASE, 0x0);
    vmwrite(GUEST_GS_BASE, 0x0);
    vmwrite(GUEST_SS_BASE, 0x0);
    vmwrite(GUEST_LDTR_BASE, 0x0);
    vmwrite(GUEST_IDTR_BASE, (size_t)state->idt.address);
    vmwrite(GUEST_GDTR_BASE, (size_t)state->gdt.address);
    vmwrite(GUEST_TR_BASE, 0x0);
    vmwrite(GUEST_CS_LIMIT, 0xffff);
    vmwrite(GUEST_DS_LIMIT, 0xffff);
    vmwrite(GUEST_ES_LIMIT, 0xffff);
    vmwrite(GUEST_FS_LIMIT, 0xffff);
    vmwrite(GUEST_GS_LIMIT, 0xffff);
    vmwrite(GUEST_SS_LIMIT, 0xffff);
    vmwrite(GUEST_LDTR_LIMIT, 0xffff);
    vmwrite(GUEST_TR_LIMIT, 0xffff);
    vmwrite(GUEST_GDTR_LIMIT, state->gdt.size);
    vmwrite(GUEST_IDTR_LIMIT, state->idt.size);

    vmwrite(HOST_ES_SELECTOR, GDT_DATA);
    vmwrite(HOST_CS_SELECTOR, GDT_CODE);
    vmwrite(HOST_SS_SELECTOR, GDT_DATA);
    vmwrite(HOST_DS_SELECTOR, GDT_DATA);
    vmwrite(HOST_FS_SELECTOR, GDT_DATA);
    vmwrite(HOST_GS_SELECTOR, GDT_DATA);

    vmwrite(GUEST_CS_ACCESS_RIGHT, CODE_ACCESS_RIGHT);
    vmwrite(GUEST_DS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    vmwrite(GUEST_ES_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    vmwrite(GUEST_FS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    vmwrite(GUEST_GS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    vmwrite(GUEST_SS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    vmwrite(GUEST_LDTR_ACCESS_RIGHT, LDTR_ACCESS_RIGHT);
    vmwrite(GUEST_TR_ACCESS_RIGHT, TR_ACCESS_RIGHT);
    vmwrite(GUEST_DR7, 0x0);
    vmwrite(GUEST_RFLAG, state->rflags);
    //used for nested virtualization
    vmwrite(VMCS_LINK_POINTER, -1ll);
    vmwrite(VMCS_FIELD_GUEST_EFER_FULL, state->efer);

    vmwrite(GUEST_INTR_STATUS, 0);
    vmwrite(GUEST_PML_INDEX, 0);
    //copy fixed values to guest
    uint64_t cr0FixedGuest = __rdmsr(IA32_VMX_CR0_FIXED0_MSR);
    uint32_t cr0FixedLo = (uint32_t)cr0FixedGuest;
    uint32_t cr0FixedHi = (uint32_t)(cr0FixedGuest >> 32);
    vmwrite(GUEST_CR0, cr0FixedLo | ((uint64_t)cr0FixedHi) << 32 | state->cr0);
    TRACE("cr3: %X", state->cr3);
    vmwrite(GUEST_CR3, state->cr3);
    uint64_t cr4FixedGuest = __rdmsr(IA32_VMX_CR4_FIXED0_MSR);
    uint32_t cr4FixedLo = (uint32_t)cr4FixedGuest;
    uint32_t cr4FixedHi = (uint32_t)(cr4FixedGuest >> 32);
    vmwrite(GUEST_CR4, cr4FixedLo | ((uint64_t)cr4FixedHi) << 32 | state->cr4);
    vmwrite(GUEST_INTERRUPTIBILITY_STATE, 0x0);
    vmwrite(GUEST_ACTIVITY_STATE, 0x0);

    vmwrite(CTLS_EPTP, (uintptr_t)g_root_pa | EPT_WB | EPT_PAGEWALK(EPT_LEVELS));

    vmwrite(GUEST_RSP, state->rsp);
    vmwrite(GUEST_RIP, state->rip);

    //TR must be non0 for vmx for some reason
    vmwrite(HOST_TR_SELECTOR, 0x18);
    vmwrite(HOST_CR3, __readcr3());

    //launch the VM, execution will resume at exit_handler
    extern void vmlaunch_first(guest_state_t *t);
    vmlaunch_first(&state->gprstate);
 
    return err;
}

void exit_handler(guest_state_t *s) {
    while (1) {
        size_t error = vmread(VM_INSTRUCTION_ERROR);
        if (error) {
            TRACE("VM entry error: %x", error);
        }
        uint32_t reason = vmread(VM_EXIT_REASON);
        if (VMEXIT_EPT_VIOLATION == reason) {
            size_t address = vmread(EPT_VIOLATION_ADDRESS);
            ept_map(address & ~(0x1000-1));
        } else {
            ASSERT(0, "unknown vmexit 0x%x", reason);
        }
        vm_resume(s);

        //VMX sets out gdt and idt limits to all 1s, so fix that
        __lgdt(g_gdt);
        __lidt(g_idt);
    }
}
