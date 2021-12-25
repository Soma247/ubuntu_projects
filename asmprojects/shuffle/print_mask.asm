;string_masks.asm
extern printf
extern print16b
section .data
section .bss
section .text
;-----------------------------------------
   global pstrlen
pstrlen:
   push rbp
   mov rbp, rsp
   sub rsp, 16
   movdqu [rbp-16], xmm0
   mov rax, -16
   pxor xmm0, xmm0
.loop:
   add rax, 16
   pcmpistri xmm0, [rdi+rax], 0x08;00001000; equal each
   jnz .loop;ends not finded
   add rax, rcx;rax = count checked bytes, rcx==bytes, checked in last;
   ;cycle
   movdqu xmm0, [rbp-16];restore xmm0
   leave
   ret
;-----------------------------------------
   global print_mask
print_mask:
   push rbp
   mov rbp, rsp
   sub rsp, 16
   call reverse_xmm0;  to direct order(0,1,2,3,4) of bytes
   pmovmskb r13d, xmm0; write byte of mask to r13d
                        ;(printf read xmm0 as float)
   movdqu [rbp-16], xmm1; save xmm1
   push rdi; string1 in rdi
   mov edi, r13d; move byte of mask
   push rdx; mask in rdx
   push rcx; flag of ends in rcx
   push rbp;
   call print16b; count of bytes in rsi from caller
   pop rbp
   pop rcx
   pop rdx
   pop rdi
   movdqu xmm1, [rbp-16]; restore xmm1
   leave
   ret
;------------------------------------------
   global reverse_xmm0
reverse_xmm0:
   section .data
      .bytereverse db 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
   section .text
      push rbp
      mov rbp, rsp
      sub rsp, 16
      movdqu [rbp-16], xmm2
      movdqu xmm2, [.bytereverse]; load mask to xmm2
      pshufb xmm0, xmm2; shuffle
      movdqu xmm2, [rbp-16]; restore xmm2
      leave
      ret
