#include <arch/intrin.h>

uint64_t __rdmsr(uint32_t msr) {
    uint32_t val1, val2;
    __asm__ __volatile__(
    "rdmsr"
    : "=a" (val1), "=d" (val2)
    : "c" (msr));
    return ((uint64_t) val1) | (((uint64_t)val2) << 32u);
}

void __wrmsr (uint32_t msr, uint64_t Value) {
    uint32_t val1 = Value, val2 = Value >> 32;
    __asm__ __volatile__ (
    "wrmsr"
    :
    : "c" (msr), "a" (val1), "d" (val2));
}

ia32_cr4_t __readcr4(void) {
    uint64_t value;
    __asm__ __volatile__ (
    "mov %%cr4, %[value]"
    : [value] "=q" (value));
    return (ia32_cr4_t) { .raw = value };
}

uint64_t __readcr3(void) {
    uint64_t value;
    __asm__ __volatile__ (
    "mov %%cr3, %[value]"
    : [value] "=q" (value));
    return value;
}

ia32_cr0_t __readcr0(void) {
    uint64_t value;
    __asm__ __volatile__ (
    "mov %%cr0, %[value]"
    : [value] "=q" (value));
    return (ia32_cr0_t) { .raw = value };
}

void __writecr4(ia32_cr4_t Data) {
    __asm__ __volatile__ (
    "mov %[Data], %%cr4"
    :
    : [Data] "q" (Data.raw)
    : "memory");
}

void __writecr0(ia32_cr0_t Data) {
    __asm__ __volatile__ (
    "mov %[Data], %%cr0"
    :
    : [Data] "q" (Data.raw)
    : "memory");
}

descriptor_t __sgdt() {
    descriptor_t res;
    asm volatile("sgdt %[gdt]": [gdt]"=m"(res));
    return res;
}

descriptor_t __sidt() {
    descriptor_t res;
    asm volatile("sidt %[idt]": [idt]"=m"(res));
    return res;
}

void __lgdt(descriptor_t gdt) {
    asm volatile("lgdt %[idt]": [idt]"=m"(gdt));
}

void __lidt(descriptor_t idt) {
    asm volatile("lidt %[idt]": [idt]"=m"(idt));
}
