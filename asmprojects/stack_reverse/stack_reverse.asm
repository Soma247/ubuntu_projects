%include "io64.inc"
extern printf
section .data
    string db "This is string to reverse",0
    strlen equ $-string-1; lenght without 0, $ is this pointer
    fmt1 db "original string is: %s",10,0
    fmt2 db "reversed string is: %s",10,0
section .bss
section .text
global CMAIN
CMAIN:
    mov rbp, rsp; for correct debugging
; print
    mov rdi, fmt1
    mov rsi, string
    xor rax, rax
    call printf
;push
    xor rax, rax
    xor r12, r12
    mov rbx, string
    mov rcx, strlen
pushloop:
    mov al, byte[rbx+r12]; ptr to next char
    push rax
    inc r12
    loop pushloop
    mov rbx, string
    mov rcx, strlen
    xor r12, r12
poploop:
    pop rax
    mov byte[rbx+r12], al
    inc r12
    loop poploop
    mov byte[rbx+r12], 0
 ;print reversed
    mov rdi, fmt2
    mov rsi, string
    xor rax, rax
    call printf
;exit
    xor rax, rax
    ret