#ifndef __VIRTDBG_GDT_H__
#define __VIRTDBG_GDT_H__

#include <stdint.h>

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

typedef struct gdt_entries {
    gdt64_entry_t null;
    gdt64_entry_t code;
    gdt64_entry_t data;
} __attribute__((packed)) gdt_entries_t;

typedef struct gdt {
    uint16_t size;
    gdt_entries_t* entries;
} __attribute__((packed)) gdt_t;

extern gdt_t g_gdt;

void init_gdt();

#endif //__VIRTDBG_GDT_H__
