#ifndef __VIRTDBG_GDT_H__
#define __VIRTDBG_GDT_H__

#include <stdint.h>
#include <virtdbg.h>

#define GDT_CODE offsetof(gdt_entries_t, code)
#define GDT_DATA offsetof(gdt_entries_t, data)

typedef struct gdt64_entry {
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt64_entry_t;

typedef struct tss_entry {
    uint16_t length;
    uint16_t base_low16;
    uint8_t  base_mid8;
    uint8_t  flags1;
    uint8_t  flags2;
    uint8_t  base_high8;
    uint32_t base_upper32;
    uint32_t reserved;
} __attribute__((packed)) tss_entry_t;

typedef struct gdt_entries {
    gdt64_entry_t null;
    gdt64_entry_t code;
    gdt64_entry_t data;
    tss_entry_t tss;
} __attribute__((packed)) gdt_entries_t;

extern descriptor_t g_gdt;

void init_gdt();

#endif //__VIRTDBG_GDT_H__
