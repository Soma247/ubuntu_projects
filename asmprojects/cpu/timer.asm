;timer.asm
extern printf
section .data 
   fmt1 db "Number of loops: %d",10,0
   fmt2 db "Elapsed cycles: %d",10,0
   loops dq 10000
section .bss
   begshi_cy resq 1
   begslo_cy resq 1
   endshi_cy resq 1
   endslo_cy resq 1
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   mov rdx, [loops]
   cpuid;barrier for serialize
   rdtsc
   mov [begshi_cy], edx
   mov [begslo_cy], eax
   call work
   rdtscp
   mov [endshi_cy], edx
   mov [endslo_cy], eax
   cpuid;barrier
   mov rdi, fmt1
   mov rsi, [loops]
   call printf

   mov rdx, [endslo_cy]
   mov rsi, [endshi_cy]
   shl rsi, 32
   or rsi, rdx; end_time in tsi
   mov r8, [begslo_cy]
   mov r9, [begshi_cy]
   shl r9, 32
   or r9, r8; beg time in r9
   sub rsi, r9
   mov rdi, fmt2
   call printf
   leave
   ret
;------------
work:
   push rbp
   mov rbp, rsp
   .loop:
      pxor xmm0, xmm0
      xor r10, r10
      xor rax, rax
      mov r12, 4
      push rcx
      pop rcx
      dec rdx
      jnz .loop
   leave
   ret
