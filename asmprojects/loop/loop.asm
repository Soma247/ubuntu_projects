%include "io64.inc"
section .data
    count dq 25;
    fmtstr db "sum is %d",10,0;
section .text
global CMAIN
CMAIN:
    mov rbp, rsp; for correct debugging
    mov rcx, [count];
    xor rax,rax; rax=0
loop_start:
    add rax,1;
    loop loop_start;
    mov rdi, fmtstr;
    mov rsi, rax;
    xor rax,rax;
    call printf;
    mov r8, 0xffa
    xor rax, rax;
    ret