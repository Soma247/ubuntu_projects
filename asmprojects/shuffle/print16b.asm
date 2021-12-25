;string_masks.asm
extern printf
section .data:
   .fmt db "%d",0
;ll n in rdi, int length in esi
; rcx, rdx, rdi caller saved
section .text:
   global print16b
print16b:
   push rbp
   mov rbp, rsp
   push rbx
   push r8
   xor r8, r8;
   mov r8d, edi; mask in r8
   mov r9, rsi; len in r9
   xor rdx, rdx
   mov dl, 16
   sub rdx, rsi; while counter>= rdx
   xor rcx, rcx
   mov cl, 15; counter
   mov rdi, .fmt
   .loop:
         xor rax, rax
         mov eax, r8d
         shr eax, cl
         xor rsi, rsi
         test eax, 1
         setnz sil
         push rcx
         push rdx
         push r8
         push r9
         mov rdi, .fmt
         call printf
         pop r9
         pop r8
         pop rdx
         pop rcx
         dec rcx
         cmp rcx, rdx
         jge .loop
   pop r8
   pop rbx
   leave
   ret
