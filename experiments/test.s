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

test:
    mov rbx, 0x1122334455667788
    cmp rdi, 0
    jne 0x6 ; true -> rbx; false -> rcx
    call rbx
continue:
    mov rdi, 1

_start:
    call test

    exit(0)