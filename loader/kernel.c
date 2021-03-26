#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include "string.h"
#include "pmm.h"
#include "vmm.h"
#include "../virtdbg/virtdbg.h"

static uint8_t stack[4096];


struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = 0
    },
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};

__attribute__((section(".stivale2hdr"), used))
struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0,
    .tags = (uintptr_t)&framebuffer_hdr_tag
};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;) {
        if (current_tag == NULL) {
            return NULL;
        }

        if (current_tag->identifier == id) {
            return current_tag;
        }

        current_tag = (void *)current_tag->next;
    }
}

uint64_t __rdmsr(uint32_t msr) {
    uint32_t val1, val2;
    __asm__ __volatile__(
    "rdmsr"
    : "=a" (val1), "=d" (val2)
    : "c" (msr));
    return ((uint64_t) val1) | (((uint64_t)val2) << 32u);
}

extern size_t base;

void _start(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_memmap *memmap_tag =
        stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    struct stivale2_struct_tag_framebuffer *fb_hdr_tag;
    fb_hdr_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
    struct stivale2_struct_tag_modules *modules_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);
    struct stivale2_module *module = &modules_tag->modules[0];

    size_t virtdbg_size = module->end - module->begin;

    pmm_init(memmap_tag->memmap, memmap_tag->entries);

    void* virtdbg_phys = pmm_allocz_aligned(virtdbg_size, 0x1000);
    uint64_t* pagemap = pmm_allocz_aligned(PAGE_SIZE, PAGE_SIZE);
    vmm_init(pagemap, memmap_tag->memmap, memmap_tag->entries);


    for (uintptr_t p = 0; p < virtdbg_size; p += PAGE_SIZE) {
        map_page(pagemap, (size_t)virtdbg_phys + p, KERNEL_OFFSET + p, 0x03);
    }
    //TODO make a stack
    void *stack = pmm_allocz_aligned(8 * 1024, 16);
    //this is where virtdbg_end should be set, all things that the debugger will have to access are allocated before this
    struct virtdbg_args args = {0};
    args.initial_guest_state = pmm_allocz_aligned(sizeof(initial_guest_state_t), 1);
    size_t allocation_end = base;

    memcpy((void*)KERNEL_OFFSET, (void*)module->begin, virtdbg_size);

    if (fb_hdr_tag == NULL) {
        for (;;) {
            asm ("hlt");
        }
    }

    uint8_t *fb_addr = (uint8_t *)fb_hdr_tag->framebuffer_addr;

    args.stolen_memory_base = (size_t)virtdbg_phys;
    args.virtdbg_end = allocation_end;
    args.stolen_memory_end = allocation_end + 0x200000;

    uint64_t cr0, cr3, cr4;

    __asm__ __volatile__ (
    "mov %%cr3, %[cr3]"
    : [cr3] "=q" (cr3));
     __asm__ __volatile__ (
    "mov %%cr0, %[cr0]"
    : [cr0] "=q" (cr0));
    __asm__ __volatile__ (
    "mov %%cr4, %[cr4]"
    : [cr4] "=q" (cr4));

    initial_guest_state_t st = {0};
    st.cr0 = cr0;
    st.cr3 = cr3;
    st.cr4 = cr4;
    descriptor_t gdtr;
    descriptor_t idtr;
    asm volatile("sgdt %[gdt]": [gdt]"=m"(gdtr));
    asm volatile("sidt %[gdt]": [gdt]"=m"(idtr));
    st.gdt = gdtr;
    st.idt = idtr;
    st.cs = 0x8;
    st.ds = 0x10;
    st.es = 0x10;
    st.fs = 0x10;
    st.gs = 0x10;
    st.ss = 0x10;
    st.rip = 0x1000;
    st.efer = __rdmsr(0xc0000080);
    st.rflags = 1 << 1;
    *(volatile uint8_t*)(0x1000) = 0xf4;
    args.initial_guest_state_count = 0;
    args.initial_guest_state[0] = st;

    for (size_t i = 0; i < 128; i++) {
        fb_addr[i] = 0xff;
    }

    void (*v)(struct virtdbg_args* a) = (void (*)())(KERNEL_OFFSET);
    asm volatile (
        "mov %[stack], %%rsp\n"
        "jmp *%[function]\n"
        "cli\n"
        "hlt\n"
        : :
          [function] "r" (v),
          [stack]    "r" (&stack),
          "D" (&args)
        : "memory" );

    for (;;) {
        asm ("hlt");
    }
}
