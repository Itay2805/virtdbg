#include "string.h"

__attribute__((naked))
void* memcpy(void* dst, void* src, size_t count) {
    asm (
        "movq %rdi, %rax\n"
        "movq %rdx, %rcx\n"
        "rep movsb\n"
        "ret"
    );
}

__attribute__((naked))
void* memset(void* dst, int value, size_t count) {
    asm (
        "movq %rdi, %r9\n"
        "movb %sil, %al\n"
        "movq %rdx, %rcx\n"
        "rep stosb\n"
        "movq %r9, %rax\n"
        "ret"
    );
}
