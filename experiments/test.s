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

_push:
    mov r10, stack_ptr
    cmp r13, 0
    cmovl r13, r10
    mov [r10 + r13 * 8], rdi
    dec r13

_pop:
    mov r10, stack_ptr
    cmp r13, stack_size
    cmovg r13, r10
    mov rdi, [r10 + r13 * 8]
    inc r13
    int 3

_start:
    mov r13, 0
    mov r13, 0

    exit(0)