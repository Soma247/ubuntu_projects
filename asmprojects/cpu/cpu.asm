;cpu.asm
extern printf
section .data
   fmt_no_sse db "This cpu does not support SSE",10,0
   fmt_sse42 db "This cpu supports SSE 4.2",10,0
   fmt_sse41 db "This cpu supports SSE 4.1",10,0
   fmt_ssse3 db "This cpu supports SSSE 3",10,0
   fmt_sse3 db "This cpu supports SSE 3",10,0
   fmt_sse2 db "This cpu supports SSE 2",10,0
   fmt_sse db "This cpu supports SSE 1",10,0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   call cpu_sse; return 1 in rax, if support
   leave
   ret

cpu_sse:
   push rbp
   mov rbp, rsp
   xor r12, r12; for sse flag
   mov eax, 1; for request cpu support flags
   cpuid
; check sse support
   test edx, 2000000h; check 25th bit(sse)
   jz sse2; if not sse
   mov r12, 1
   xor rax, rax
   mov rdi, fmt_sse
   push rcx; changing by printf
   push rdx; save result of cpuid
   call printf; sse
   pop rdx; restore result of cpuid
   pop rcx
sse2:
   test edx, 4000000h; check 26th bit(SSE2)
   jz sse3; if not sse2
   mov r12, 1
   xor rax, rax
   mov rdi, fmt_sse2
   push rcx
   push rdx
   call printf;sse2
   pop rdx
   pop rcx
sse3:
   test ecx, 1;check zero bit(SSE3)
   jz ssse3;if not sse3
   mov r12, 1
   xor rax, rax
   mov rdi, fmt_sse3
   push rcx
   push rdx
   call printf; sse3
   pop rdx
   pop rcx
ssse3:
   test ecx, 9h; check bit #0 (sse3)
   jz sse41;if ssse3 is not supported
   mov r12, 1
   xor rax, rax
   mov rdi, fmt_ssse3
   push rcx
   call printf
   pop rcx
sse41:
   test ecx, 80000h; check bit #19(sse 4.1)
   jz sse42; if not sse41
   mov r12, 1
   xor rax, rax
   mov rdi, fmt_sse41
   push rcx
   call printf
   pop rcx
sse42:
   test ecx, 100000h; check bit #20(sse 4.2)
   jz wrapup; if not sse42
   mov r12, 1
   xor rax, rax
   mov rdi, fmt_sse42
   push rcx
   call printf
   pop rcx
wrapup:
   cmp r12, 1
   je sse_ok; if sse is supported
   mov rdi, fmt_no_sse
   xor rax, rax
   call printf
   jmp the_exit
sse_ok:
   mov rax, r12; return 1, supported sse
the_exit:
   leave
   ret
