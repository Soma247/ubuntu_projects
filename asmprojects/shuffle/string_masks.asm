;string_masks.asm
extern printf
extern print_mask
extern pstrlen
extern reverse_xmm0
section .data
   string1 db "qdacdekkfijlmdoza"
           db "becdfgdklmdddaf"
           db "fffffffdedeee",10,0
   string2 db "e",0
   string3 db "a",0
   fmt     db "Find all the characters '%s' "
           db "and '%s' in:",10,0
   fmt_oc  db "I found %ld characters '%s' and '%s'",10,0
   NL      db 10,0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;print chars to search
   mov rdi, fmt
   mov rsi, string2
   mov rdx, string3
   xor rax, rax
   call printf
;print string
   mov rdi, string1
   xor rax, rax
   call printf
;searcing, print the mask
   mov rdi, string1
   mov rsi, string2
   mov rdx, string3
   call pcharsrch
;print results
   mov rdi, fmt_oc
   mov rsi, rax
   mov rdx, string2
   mov rcx, string3
   call printf
   leave
   ret
;--------find packed character-------------
pcharsrch:
   push rbp
   mov rbp, rsp
   sub rsp, 16; reserve 16 bytes in stack
   xor r12, r12;encounting count
   xor rcx, rcx;for ends indication
   xor rbx, rbx;for addr
   mov rax, -16;for bytes counter (no flag ZF)
;load chars to find
   pxor xmm1, xmm1
   pinsrb xmm1, byte[rsi], 0;char 'e' to #0
   pinsrb xmm1, byte[rdx], 1; char 'a' to #1
.loop:
   add rax, 16;no ZF flag after pcmpistrm, now ZF==1
   mov rsi, 16;if not ends, return this 16
   movdqu xmm2, [rdi+rbx]; load 16 bytes to xmm2
   pcmpistrm xmm1, xmm2, 40h; 01000000 - equal each | mask returned in xmm0
   setz cl; if ZF(ends finded) mov cl, 1
;if ends, compute position
   cmp cl, 0
   je .print;not ends
   ;ends finded, <16 bytes left, adr of str in rdi, checked bytes counter
   ; in rbx
   add rdi, rbx; addr of first of left bytes in rdi
   push rcx;caller saved
   call pstrlen; return length in rax with ends
   pop rcx; restore rcx
   dec rax;without ends
   mov rsi, rax; lenhth left bytes in mask
.print:
   call print_mask
;continue compute count of occurences
   popcnt r13d, r13d; compute count of bits==1
   add r12d, r13d; save count of occurences
   or cl, cl;ends finded?
   jnz .exit
   add rbx, 16;preparing next 16 bytes
   jmp .loop
.exit:
   mov rdi, NL
   call printf
   mov rax, r12; count of occurences
   leave
   ret
