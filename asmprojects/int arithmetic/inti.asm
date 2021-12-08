%include "io64.inc"
extern printf
section .data
    number1 dq 128; some numbers
    number2 dq 10;  for arithmetic
    neg_num dq -128;
    fmt     db "The numbers are %ld and %ld, neg is %ld",10,0
    fmtint  db "%s %ld",10,0
    sumi    db "the sum is",0
    difi    db "The difference is",0
    inci    db "Number 1 incremented",0
    deci    db "Number 1 decremented",0
    sali    db "Number 1 shift left 2(x4)",0
    sari    db "Number 1 shift right 2(/4)",0
    sariex  db "Number 1 shift right 2(/4)" 
             db "with sign extention",0
    multi   db "The product is",0
    divi    db "The integer quotient is",0
    remi    db "The modulo is",0
    
    testn1 dq 12345678901234567
    testn2 dq 100
    testn3 dq 10000
section .bss
    resulti resq 1
    modulo  resq 1
section .text
global CMAIN
CMAIN:
    mov rbp, rsp; for correct debugging
; print
    mov rdi, fmt
    mov rsi, [number1]
    mov rdx, [number2]
    mov rcx, [neg_num]
    xor rax, rax
    call printf
;summ
    mov rax, [number1]
    add rax, [number2]
    mov [resulti],rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, sumi
    mov rdx, [resulti]
    call printf
 ;sub
    mov rax, [number1]
    sub rax, [number2]
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, difi
    mov rdx, [resulti]
    call printf
;incrementing
    mov rax, [number1]
    inc rax
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, inci
    mov rdx, [resulti]
    call printf;
; decrementing
    mov rax, [number1]
    dec rax
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, deci
    mov rdx, [resulti]
    call printf;  
; shift left
    mov rax, [number1]
    sal rax, 2
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, sali
    mov rdx, [resulti]
    call printf;  
; shift right
    mov rax, [number1]
    sar rax, 2
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, sari
    mov rdx, [resulti]
    call printf;  
; shift right signed
    mov rax, [neg_num]
    sar rax, 2
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, sariex
    mov rdx, [resulti]
    call printf;  
;multiply 
    mov rax, [number1]
    imul qword [number2]; first number in rax, second in parameter, result in rdx:rax 128 bit
    mov [resulti], rax
    xor rax, rax
    mov rdi, fmtint
    mov rsi, multi
    mov rdx, [resulti]
    call printf;
    
    mov rax, [testn1]
    imul qword [testn2];ok, result fited to rax, rdx=0
    mov rax, [testn1]
    imul qword [testn3]; warning, lower 64 bits of result in rax(<0), higher 64 in rdx  
;integer divide
    mov rax, [number1]
    xor rdx, rdx
    idiv qword [number2]; dividend in rdx:rax(128) 
                        ;!!!DONT FORGET CLEAR RDX, if u dividing 64!!!
                        ;, divider in param, integer result in rax, modulo in rdx
    mov [resulti], rax
    mov [modulo], rdx
    xor rax, rax
    mov rdi, fmtint
    mov rsi, divi
    mov rdx, [resulti]
    call printf;  
    mov rdi, fmtint
    mov rsi, remi
    mov rdx, [modulo]
    xor rax, rax
    call printf
;exit
    xor rax, rax
    ret