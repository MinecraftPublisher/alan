section .data
msg db 'Hello World!', 0Ah     ; assign msg variable with your message string

section .text
global  _start

bits 64

%macro sys 1
    mov rax, %1
    int 0x80
%endmacro

%define cwrite 4
%define cexit 1

%macro exit 1
    mov rbx, %1
    sys(cexit)
%endmacro

%define stack_ptr 0x1122334455667788
%define stack_size 8192

_push_restore:
    mov r10, stack_ptr
    cmp r13, stack_size
    cmovg r13, r10
    mov [r10 + r13 * 8], r11
    inc r13

_pop_restore:
    mov r10, stack_ptr
    dec r13
    cmp r13, 0
    cmovl r13, r10
    mov r11, [r10 + r13 * 8]

_push:
    mov r10, stack_ptr
    cmp r11, stack_size
    cmovg r11, r10
    mov [r10 + r11 * 8], rdi
    inc r11

_pop:
    mov r10, stack_ptr
    dec r11
    cmp r11, 0
    cmovl r11, r10
    mov rdi, [r10 + r11 * 8]

_pre_drop:
    mov r12, rdi
    mov rdi, r12

_start:
    mov [rsi], rdi

    exit(0)