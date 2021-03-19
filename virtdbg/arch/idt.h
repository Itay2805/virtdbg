#ifndef __VIRTDBG_IDT_H__
#define __VIRTDBG_IDT_H__

#include <stdint.h>

#include <util/except.h>
#include <util/list.h>
#include <virtdbg.h>

#include "intrin.h"

#define IDT_TYPE_TASK           0x5
#define IDT_TYPE_INTERRUPT_16   0x6
#define IDT_TYPE_TRAP_16        0x7
#define IDT_TYPE_INTERRUPT_32   0xE
#define IDT_TYPE_TRAP_32        0xF

typedef struct idt_entry {
    uint64_t handler_low : 16;
    uint64_t selector : 16;
    uint64_t ist : 3;
    uint64_t _zero1 : 5;
    uint64_t gate_type : 4;
    uint64_t _zero2 : 1;
    uint64_t ring : 2;
    uint64_t present : 1;
    uint64_t handler_high : 48;
    uint64_t _zero3 : 32;
} __attribute__((packed)) idt_entry_t;

extern descriptor_t g_idt;

typedef struct exception_context {
    uint64_t ds;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t int_num;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    ia32_rflags_t rflags;
    uint64_t rsp;
    uint64_t ss;
} exception_context_t;

void init_idt();

typedef struct exception_handler {
    list_entry_t link;
    err_t (*handle)(exception_context_t* ctx, bool* handled);
} exception_handler_t;

void hook_exception_handler(exception_handler_t* handler);

typedef struct interrupt_handler {
    list_entry_t link;
    void* ctx;
    err_t (*handle)(int vector, void* ctx, bool* handled);
} interrupt_handler_t;

void hook_interrupt_handler(interrupt_handler_t* handler, int vector);
#endif
