;area.asm
extern printf;
section .data
   radius dq 9.0 ; constexpr
   pi dq 3.14
   fmt db "The area of the circle with radius = %f is %f",10,0
section .bss
section .text
   global main

main:
   push rbp;
   mov rbp, rsp;
   call area
   xor rax,rax
   mov rax, 2
   mov rdi, fmt
   movsd xmm0, [radius]; first arg to printf
   call printf ; quit
   leave
   ret

area:
   push rbp
   mov rbp, rsp
   movsd xmm1, [radius]
   mulsd xmm1, [radius]
   mulsd xmm1, [pi]
   leave
   ret
