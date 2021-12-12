;asum.asm
section .data
section .bss
section .text
   global asum
asum:
   section .text
      push rbp
      mov rbp, rsp
      mov rcx, rsi
      mov rbx, rdi
      xor r12, r12
      movsd xmm0, qword[rbx]; *begin to xmm0
      dec rcx

   sloop:
      inc r12
      addsd xmm0, qword[rbx+r12*8]; add each
      loop sloop
   leave
   ret

   
