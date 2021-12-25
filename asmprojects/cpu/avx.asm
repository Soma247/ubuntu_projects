;avx.asm
extern printf
section .data
   fmt_no_avx db "This cpu doesn't support avx.",10,0
   fmt_avx db "This cpu supports avx.",10,0
   fmt_no_avx2 db "This cpu doesn't support avx2.",10,0
   fmt_avx2 db "This cpu supports avx2.",10,0
   fmt_no_avx512 db "This cpu doesn't support avx512.",10,0
   fmt_avx512 db "This cpu supports avx512.",10,0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   mov eax, 1; request cpu specification flags
   cpuid
   mov eax, 28
   bt ecx, eax; check #28 bit in ecx
   jnc no_avx
   xor rax, rax
   mov rdi, fmt_avx
   call printf
   ;---------
   mov eax, 7
   xor ecx, ecx
   cpuid; request cpu flags
   mov eax, 5
   bt ebx, eax;check #5 bit in ebx
   jnc to_exit
   xor rax, rax
   mov rdi, fmt_avx2
   call printf
   ;-----------
   mov eax, 7
   xor ecx, ecx
   cpuid
   mov eax, 16
   bt ebx, eax; check #16 bit in ebx
   jnc no_avx512
   xor rax, rax
   mov rdi, fmt_avx512
   call printf
   jmp to_exit
no_avx:
   xor rax, rax
   mov rdi, fmt_no_avx
   call printf
   xor rax, rax
   jmp to_exit
no_avx2:
   xor rax, rax
   mov rdi, fmt_no_avx2
   call printf
   xor rax, rax
   jmp to_exit
no_avx512:
   xor rax, rax
   mov rdi, fmt_no_avx512
   call printf
   xor rax, rax
to_exit:
   leave
   ret


