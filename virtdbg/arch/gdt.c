#include "gdt.h"

static gdt_entries_t m_entries = {
    {   // null descriptor
        .limit          = 0x0000,
        .base_low       = 0x0000,
        .base_mid       = 0x00,
        .access         = 0b00000000,
        .granularity    = 0b00000000,
        .base_high      = 0x00
    },
    {   // kernel code
        .limit          = 0x0000,
        .base_low       = 0x0000,
        .base_mid       = 0x00,
        .access         = 0b10011010,
        .granularity    = 0b00100000,
        .base_high      = 0x00
    },
    {   // kernel data
        .limit          = 0x0000,
        .base_low       = 0x0000,
        .base_mid       = 0x00,
        .access         = 0b10010010,
        .granularity    = 0b00000000,
        .base_high      = 0x00
    },
    {
        .length         = 104,
        .base_low16     = 0,
        .base_mid8      = 0,
        .flags1         = 0b10001001,
        .flags2         = 0,
        .base_high8     = 0,
        .base_upper32   = 0,
        .reserved       = 0
    }
};

descriptor_t g_gdt = {
    .size = sizeof(gdt_entries_t) - 1,
    .address = (uint64_t)&m_entries
};

void init_gdt() {
    asm volatile (
        "lgdt %0\n"
        "movq %%rsp, %%rax\n"
        "pushq $16\n"
        "pushq %%rax\n"
        "pushfq\n"
        "pushq $8\n"
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "iretq\n"
        "1:\n"
        "movw $16, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        :
        : "m"(g_gdt)
        : "memory", "rax"
    );
}
