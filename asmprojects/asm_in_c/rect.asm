;rect.asm
section .data
section .bss
section .text
   global rarea
rarea:
   section .text
      push rbp
      mov rbp, rsp
      mov rdi, rax
      imul rsi
      leave
      ret
;-----------------
   global rcircum
rcircum:
   section .text
      push rbp
      mov rbp, rsp
      mov rdi, rax
      add rax, rsi
      imul rax, 2
      leave
      ret
;------------------
