;alive.asm
extern printf; extern declaration c function -no-pie required
section .data
    msg1 db "Hello, World!",10,0 ; string with /n/0
    msg1len equ $-msg1-1;length msg1 = &msg1len -&msg1 -1 (without /0)
    msg2 db "Alive and Kicking!",10,0;
    fmtstr db "%s",10,0;
    radius dd 357;
    fmtint db "%d",10,0;
    pi dq 3.14;
    fmtfloat db "%lf",10,0;
section .bss
section .text
    global main
main:
    push rbp; function prologue(base pointer to fstack bottom to stack)
    mov rbp, rsp; function prologue (set ptr to stack bottom as base pointer
    mov rax, 1; write
    mov rdi, 1; to stdout
    mov rsi, msg1;
    mov rdx, msg1len;
    syscall;
    mov rax, 0; non-floating point flag
    mov rdi, fmtstr; format cstring(with /0)
    mov rsi, msg2; ptr to data(cstring with /0)
    call printf;
    mov rax, 0;
    mov rdi, fmtint;
    mov rsi, [radius]; like a *ptr in c (data from adr in ptr)
    call printf;
    mov rax, 1;
    mov rdi, fmtfloat;
    movq xmm0, [pi];
    call printf;
    mov rsp, rbp; epilogue function
    pop rbp; epilogue
    ret;{60->rax, 0->rdi, syscall}