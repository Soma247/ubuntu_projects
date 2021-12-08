extern printf
section .data
    num1 dq 9.0
    num2 dq 73.0
    fmt  db "The numbers are %f and %f",10,0
    fmtfloat db "%s %f",10,0
    fmtsum db "The sum of %f and %f is %f",10,0
    fmtsub db "The difference of %f and %f is %f",10,0
    fmtmul db "The product of %f and %f is %f",10,0
    fmtdiv db "The division of %f and %f is %f",10,0
    fmtsqr db "The squareroot of %f is %f",10,0
section .text
global main
main:
    push rbp;
    mov rbp, rsp; for correct debugging
;print  
    movsd xmm0, [num1]; movss, if float, movsd if double
    movsd xmm1, [num2]
    xor rax, rax
    mov rax, 2
    mov rdi, fmt
    call printf
;summ
   movsd xmm2,[num1]
   addsd xmm2, [num2]
   movsd xmm0,[num1]
   movsd xmm1, [num2]
   mov rdi, fmtsum
   xor rax,rax
   mov rax, 3
   call printf
;sub
   movsd xmm2, [num1]
   subsd xmm2, [num2]
   movsd xmm0, [num1]
   movsd xmm1, [num2]
   xor rax,rax
   mov rax,3
   mov rdi, fmtsub
   call printf
;mul
   movsd xmm2,[num1]
   mulsd xmm2,[num2]
   movsd xmm0, [num1]
   movsd xmm1, [num2]
   xor rax,rax
   mov rax, 3
   mov rdi, fmtmul
   call printf
;div
   movsd xmm2,[num2]
   divsd xmm2,[num1]
   movsd xmm0, [num2]
   movsd xmm1, [num1]
   xor rax,rax
   mov rax, 3
   mov rdi, fmtdiv
   call printf
;sqrt
   movsd xmm0, [num1]
   sqrtsd xmm1,[num1]
   xor rax, rax
   mov rax, 2
   mov rdi, fmtsqr
   call printf


;exit
    xor rax, rax
    mov rsp, rbp
    pop rbp
    ret
