;area.asm
extern printf;
extern c_area
extern c_circum
extern r_area
extern r_circum
global pi
section .data
   pi dq 3.141592654
   radius dq 10.0
   side1 dq 4
   side2 dq 5
   fmtf db "%s %f",10,0
   fmti db "%s %d",10,0
   ca db "The circle area is ",0
   cc db "The circle circumference is ",0
   ra db "The rectangle area is ",0
   rc db "The rectangle circumference is ",0
section .bss
section .text
   global main
main:
   push rbp;
   mov rbp, rsp;
;circle area
   movsd xmm0, qword[radius]
   call c_area
   mov rdi, fmtf
   mov rsi, ca
   mov rax, 1
   call printf
; circle circumstance
   movsd xmm0, qword[radius]
   call c_circum
   mov rdi, fmtf
   mov rsi, cc
   mov rax, 1
   call printf
;rect area
   mov rdi, [side1]
   mov rsi, [side2]
   call r_area
   mov rdi, fmti
   mov rsi, ra
   mov rdx, rax
   xor rax, rax
   call printf
;rect circumstance
   mov rdi, [side1]
   mov rsi, [side2]
   call r_circum
   mov rdi, fmti
   mov rsi, rc
   mov rdx, rax
   xor rax,rax
   call printf
   leave
   ret
