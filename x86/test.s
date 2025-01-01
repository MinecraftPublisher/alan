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

_start:
    ; move 64-bit value into register
    mov rdi, 0x1122334455667788
    mov rdi, [rdi]
    mov [rbx], rdi
    ; ret

    exit(0)