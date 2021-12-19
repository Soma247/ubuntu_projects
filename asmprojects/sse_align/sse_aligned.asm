;sse_aligned.asm
extern printf
section .data
   dummy db 13;memory is not aligned after this variable
align 16; befor each datablock, required aligning
   spvector1 dd 1.1
             dd 2.2
             dd 3.3
             dd 4.4
   spvector2 dd 1.1
             dd 2.2
             dd 3.3
             dd 4.4
   dpvector1 dq 1.1
             dq 2.2
   dpvector2 dq 3.3
             dq 4.4
   fmt1 db "Single precision vector 1: %f %f %f %f", 10,0
   fmt2 db "Single precision vector 2: %f %f %f %f", 10,0
   fmt3 db "Sum of single precision vectors: %f %f %f %f", 10,0
   fmt4 db "Double precision vector 1: %f %f",10,0
   fmt5 db "Double precision vector 2: %f %f",10,0
   fmt6 db "Sum of double precision vectors: %f %f",10,0
section .bss
alignb 16
   spvec_result resd 4
   dpvec_result resq 4
section .text
   global main
main:
   push rbp
   mov rbp, rsp
; sum single precision vectors
   mov rsi, spvector1
   mov rdi, fmt1
   call printspfp
   mov rsi, spvector2
   mov rdi, fmt2
   call printspfp
   movaps xmm0, [spvector1]
   addps xmm0, [spvector2]
   movaps [spvec_result], xmm0
   mov rsi, spvec_result
   mov rdi, fmt3
   call printspfp
; sum double precision vectors
   mov rsi, dpvector1
   mov rdi, fmt4
   call printdpfp
   mov rsi, dpvector2
   mov rdi, fmt5
   call printdpfp
   movapd xmm0, [dpvector1]
   addpd xmm0, [dpvector2]
   movapd [dpvec_result], xmm0
   mov rsi, dpvec_result
   mov rdi, fmt6
   call printdpfp
   leave
   ret
;--------------------------------
printspfp:
   push rbp
   mov rbp, rsp
   movss xmm0, [rsi];load first element
   cvtss2sd xmm0, xmm0; convert to double precision for printf
   movss xmm1, [rsi+4];load second element
   cvtss2sd xmm1, xmm1
   movss xmm2, [rsi+8]
   cvtss2sd xmm2, xmm2
   movss xmm3, [rsi+12]
   cvtss2sd xmm3, xmm3
   mov rax, 4
   call printf
   leave
   ret
;--------------------------------
printdpfp:
   push rbp
   mov rbp, rsp
   movsd xmm0, [rsi]
   movsd xmm1, [rsi+8]
   mov rax, 2
   call printf
   leave
   ret

