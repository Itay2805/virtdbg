%define GUEST_RIP 0x0000681E
%define HOST_RIP  0x00006c16
%define HOST_RSP  0x00006c14
%define GUEST_RSP 0x0000681C

global vmlaunch_first
global  vm_resume
extern exit_handler

section .text
vmlaunch_first:
push rdi
; set host rip and rsp
mov rcx, HOST_RSP
vmwrite rcx, rsp
mov rcx, GUEST_RSP
vmwrite rcx, rsp
mov rcx, HOST_RIP
mov rsi, hexit
vmwrite rcx, rsi
mov rcx, GUEST_RIP
mov rsi, cont
vmwrite rcx, rsi

mov rdi, [rsp]

mov rbx, [rdi + 8]
mov rcx, [rdi + 16]
mov rdx, [rdi + 24]
mov rsi, [rdi + 32]
mov rbp, [rdi + 48]
mov r8,  [rdi + 56]
mov r9,  [rdi + 64]
mov r10, [rdi + 72]
mov r11, [rdi + 80]
mov r12, [rdi + 88]
mov r13, [rdi + 96]
mov r14, [rdi + 104]
mov r15, [rdi + 112]
mov rax, [rdi]
mov rdi, [rdi + 40]
vmlaunch

hexit:
push rdi
mov rdi, [rsp + 8]
mov [rdi], rax
mov [rdi + 8], rbx
mov [rdi + 16], rcx
mov [rdi + 24], rdx
mov [rdi + 32], rsi
mov [rdi + 48], rbp
mov [rdi + 56], r8
mov [rdi + 64], r9
mov [rdi + 72], r10
mov [rdi + 80], r11
mov [rdi + 88], r12
mov [rdi + 96], r13
mov [rdi + 104], r14
mov [rdi + 112], r15
pop r8
pop r9
mov [r9 + 40], r8
pop rdi
jmp exit_handler

cont:
hlt
jmp $


vm_resume:
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
push rbp
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15

push rdi
mov rcx, HOST_RSP
mov rdx, HOST_RIP

vmwrite rcx, rsp
mov rdi, res
vmwrite rdx, rdi

mov rdi, [rsp]

mov rbx, [rdi + 8]
mov rcx, [rdi + 16]
mov rdx, [rdi + 24]
mov rsi, [rdi + 32]
mov rbp, [rdi + 48]
mov r8,  [rdi + 56]
mov r9,  [rdi + 64]
mov r10, [rdi + 72]
mov r11, [rdi + 80]
mov r12, [rdi + 88]
mov r13, [rdi + 96]
mov r14, [rdi + 104]
mov r15, [rdi + 112]
mov rax, [rdi]
mov rdi, [rdi + 40]
vmresume
res:
push rdi
mov rdi, [rsp + 8]
mov [rdi], rax
mov [rdi + 8], rbx
mov [rdi + 16], rcx
mov [rdi + 24], rdx
mov [rdi + 32], rsi
mov [rdi + 48], rbp
mov [rdi + 56], r8
mov [rdi + 64], r9
mov [rdi + 72], r10
mov [rdi + 80], r11
mov [rdi + 88], r12
mov [rdi + 96], r13
mov [rdi + 104], r14
mov [rdi + 112], r15
pop r8
pop r9
mov [r9 + 40], r8

pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rbp
pop rdi
pop rsi
pop rdx
pop rcx
pop rbx
pop rax

ret
