;avx_unaligned.asm
extern printf
section .data
;single precision
   spvector1 dd 1.1
             dd 2.1
             dd 3.1
             dd 4.1
             dd 5.1
             dd 6.1
             dd 7.1
             dd 8.1
   spvector2 dd 1.2
             dd 2.2
             dd 3.2
             dd 4.2
             dd 5.2
             dd 6.2
             dd 7.2
             dd 8.2
;double precision
   dpvector1 dq 1.1
             dq 2.2
             dq 3.3
             dq 4.4
   dpvector2 dq 5.5
             dq 6.6
             dq 7.7
             dq 8.8
   fmt1 db "Single precision vector 1:",10,0
   fmt2 db 10,"Single precision vector 2:",10,0
   fmt3 db 10,"Sum of single precision vectors:",10,0
   fmt4 db 10,"Double precision vector 1:",10,0
   fmt5 db 10,"Double precision vector 2:",10,0
   fmt6 db 10,"Sum of double precision vectors:",10,0
section .bss
   spvec_result resd 8
   dpvec_result resq 4
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;float
   vmovups ymm0, [spvector1];load unaligned 256 bit vector from addr to ymm0
   vextractf128 xmm2, ymm0, 0; extract 0-127 bits(lower half)
                              ;from ymm0 to xmm2
   vextractf128 xmm2, ymm0, 1; extract 128-255 bits(higher half)
                              ;from ymm0 to xmm2
   vmovups ymm1, [spvector2];load to ymm1
   vaddps ymm2, ymm0, ymm1; summ ymm0+ymm1 ->ymm2
   vmovups [spvec_result], ymm2
; print
   mov rdi, fmt1
   call printf
   mov rsi, spvector1
   call printspfpv
   mov rdi, fmt2
   call printf
   mov rsi, spvector2
   call printspfpv
   mov rdi, fmt3
   call printf
   mov rsi, spvec_result
   call printspfpv
;double
   vmovups ymm0, [dpvector1]
   vmovups ymm1, [dpvector2]
   vaddpd ymm2, ymm0, ymm1
   vmovupd [dpvec_result], ymm2
;print
   mov rdi, fmt4
   call printf
   mov rsi, dpvector1
   call printdpfpv
   mov rdi, fmt5
   call printf
   mov rsi, dpvector2
   call printdpfpv
   mov rdi, fmt6
   call printf
   mov rsi, dpvec_result
   call printdpfpv
   leave
   ret
;----------------------------
printspfpv:
section .data
   .NL db 10,0
   .fmt1 db "%.1f, ",0
section .text
   push rbp
   mov rbp, rsp
   push rcx
   push rbx
   mov rcx, 8; count of numbers
   xor rbx, rbx
   mov rax, 1
   .loop:
      movss xmm0, [rsi+rbx]; next float to xmm0
      cvtss2sd xmm0, xmm0; convert to double
      mov rdi, .fmt1
      push rsi
      push rcx
      call printf; frint this float
      pop rcx
      pop rsi
      add rbx, 4; to next float(+4 bytes)
      loop .loop
   xor rax, rax
   mov rdi, .NL
   call printf
   pop rbx
   pop rcx
   leave
   ret
;-----------------------------
printdpfpv:
section .data
   .NL db 10,0
   .fmt db "%.1f, %.1f, %.1f, %.1f",0
section .text
   push rbp
   mov rbp, rsp
   mov rdi, .fmt
   mov rax, 4;4 doubles
   movsd xmm0, [rsi]
   movsd xmm1, [rsi+8]
   movsd xmm2, [rsi+16]
   movsd xmm3, [rsi+24]
   call printf
   mov rdi, .NL
   call printf
   leave
   ret
