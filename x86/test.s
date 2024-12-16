section .data
msg db 'Hello World!', 0Ah     ; assign msg variable with your message string

section .text
global  _start

%macro sys 1
    mov eax, %1
    int 0x80
%endmacro

%define cwrite 4
%define cexit 1

%macro exit 1
    mov ebx, %1
    sys(cexit)
%endmacro

_start:
    mov edx, 1
    mov eax, 2
    mov rax, rbx

    exit(0)