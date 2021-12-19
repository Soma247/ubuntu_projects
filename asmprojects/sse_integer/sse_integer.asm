;sse_integer.asm
extern printf
section .data
   dummy db 13
align 16
   pdivec1 dd 1
           dd 2
           dd 3
           dd 4 
   pdivec2 dd 5
           dd 6
           dd 7
           dd 8
   fmt1 db "Packed integer vector 1: %d %d %d %d",10,0
   fmt2 db "Packed integer vector 2: %d %d %d %d",10,0
   fmt3 db "Sum of packed int vecs : %d %d %d %d",10,0
   fmt4 db "Reverse of Sum vector  : %d %d %d %d",10,0
section .bss
align 16
   pdivec_result resd 4
   pdivec_other  resd 4
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;print v1
   mov rsi, pdivec1
   mov rdi, fmt1
   call printpdi
;print v2
   mov rsi, pdivec2
   mov rdi, fmt2
   call printpdi
;summ 2 vectors with aligned integer double precision
   movdqa xmm0, [pdivec1]
   paddd  xmm0, [pdivec2]
;save result in memory
   movdqa [pdivec_result], xmm0
;print result
   mov rsi, pdivec_result
   mov rdi, fmt3
   call printpdi
;load to xmm3
   movdqa xmm3, [pdivec_result]
;extract packed values from xmm3
   pextrd eax, xmm3, 0;index of value in pack
   pextrd ebx, xmm3, 1
   pextrd ecx, xmm3, 2
   pextrd edx, xmm3, 3
;inject values into xmm0 with reversed order
   pinsrd xmm0, eax, 3
   pinsrd xmm0, ebx, 2
   pinsrd xmm0, ecx, 1
   pinsrd xmm0, edx, 0
;print reversed vector
   movdqa [pdivec_other], xmm0
   mov rsi, pdivec_other
   mov rdi, fmt4
   call printpdi
   leave
   ret
;-----------------------------
printpdi:
   push rbp
   mov rbp, rsp
   movdqa xmm0, [rsi];mov pack to xmm0
   pextrd esi, xmm0, 0;hidher 32 bits of rsi filled zeroes
   pextrd edx, xmm0, 1
   pextrd ecx, xmm0, 2
   pextrd r8d, xmm0, 3
   xor rax, rax
   call printf;fmt in rdi already
   leave
   ret
