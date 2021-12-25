extern printf
section .data
   .fmt db "%f",9,"%f",9,"%f",9,"%f",10,0
section .text
   global printm4x4

printm4x4:
   push rbp
   mov rbp, rsp
   push rbx;
   push r15;callee saved registers
   mov rdi, .fmt
   mov rcx, 4
   xor rbx, rbx;counter of rows
   .loop:
      movsd xmm0, [rsi+rbx]
      movsd xmm1, [rsi+rbx+8]
      movsd xmm2, [rsi+rbx+16]
      movsd xmm3, [rsi+rbx+24]
      mov rax, 4; 4 float values
      push rcx
      push rsi
      push rdi;callee saved
      xor r15, r15
      test rsp, 0xf;stack aligned?
      setnz r15b;set if not
      shl r15, 3;*8
      sub rsp, r15;-0 or -8
      call printf
      add rsp, r15;restore stack pointer
      pop rdi
      pop rsi
      pop rcx
      add rbx, 32;next row
      loop .loop
   pop r15
   pop rbx
   leave
   ret
