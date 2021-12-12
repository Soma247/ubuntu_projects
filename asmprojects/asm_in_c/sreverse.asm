;sreverse.asm
section .data
section .bst
section .text
   global sreverse
sreverse:
   section .text
      push rbp
      mov rbp, rsp
   ;pushing:
      mov rcx, rsi; length
      mov rbx, rdi; ptr
      xor r12, r12; counter
   pushloop:
      mov rax, qword[rbx+r12]; read byte from string to rax
      push rax; push qword with lower byte from string to stack
      inc r12
      loop pushloop
   ;popping
      mov rcx, rsi
      mov rbx, rdi
      xor r12, r12
   poploop:
      pop rax; pop qword with char from stack
      mov byte[rbx+r12], al; write char to string
      inc r12
      loop poploop
      mov rax, rdi;ptr to cstring
      leave
      ret
      
