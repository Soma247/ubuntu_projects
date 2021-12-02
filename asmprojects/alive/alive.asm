;alive.asm
section .data
    msg1 db "Hello, World!",10,0 ; string with /n/0
    msg1len equ $-msg1-1;length msg1 = &msg1len -&msg1 -1 (without /0)
    msg2 db "Alive and Kicking!",10,0
    msg2len equ $-msg2-1;length msg2
    radius dq 357
    pi dq 3.14
section .bss
section .text
    global main
main:
    push rbp; function prologue
    mov rbp, rsp; function prologue
    mov rax, 1; write
    mov rdi, 1; to stdout
    mov rsi, msg1;
    mov rdx, msg1len;
    syscall;
    mov rax, 1;
    mov rdi, 1;
    mov rsi, msg2;
    mov rdx, msg2len;
    syscall;
    mov rsp, rbp; epilogue function
    pop rbp; epilogue
    mov rax, 60; exit proc
    mov rdi, 0; exit_success
    syscall; 
