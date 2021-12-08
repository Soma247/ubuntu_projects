extern printf
section .data
   null db "0"
   one db "1"
section .bss
   temp resb 66
section .text
   global printbits
printbits:
   push rbp
   mov rbp, rsp
   push rcx
   xor rcx, rcx
   mov rax, temp
   mov byte[rax+64], 10
   mov byte[rax+65], 0
   mov rcx, 64
   mov bh, byte[one]
   mov bl, byte[null]
noneqnull:
   sar rdi, 1
   jnc isnul
   mov byte[rax+rcx-1], bh
   jmp continue
isnul:
   mov byte[rax+rcx-1], bl
continue:
   loop noneqnull
   mov rdi, rax
   xor rax, rax
   call printf
   pop rcx
   mov rsp, rbp
   pop rbp
   ret




