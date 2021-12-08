;aligned.asm
extern printf
section .data
    fmt  db "Two times pi equal %.14f",10,0
    pi dq 3.14259265358979
section .text
global main
;f3------------------------------------
func3:
   push rbp
   movsd xmm0, [pi]
   addsd xmm0, [pi]
   mov rdi, fmt
   mov rax, 1
   call printf
   pop rbp
   ret
;f2-----------------------------------
func2:
   push rbp
   call func3
   pop rbp
   ret
;f3------------------------------------
func1:
   push rbp
   call func2
   pop rbp
   ret
;main-----------------------------------
main:
    push rbp;
    call func1
    pop rbp
    ret
