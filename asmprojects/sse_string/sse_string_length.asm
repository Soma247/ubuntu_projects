;sse_string_length.asm
extern printf
section .data
;template      0123456789abcdef0123456789abcdef0123456789abcd  e
;template      1234567890123456789012345678901234567890123456  7
   string1 db "The quick brown fox jumps over the lazy river",0
   fmt1    db "This is our string: %s",10,0
   fmt2    db "Our string is %d characters long.",10,0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   mov rdi, fmt1
   mov rsi, string1
   xor rax, rax
   call printf
   mov rdi, string1
   call pstrlen
   mov rdi, fmt2
   mov rsi, rax
   xor rax, rax
   call printf
   leave
   ret
;strlen------------------
pstrlen:
   push rbp
   mov rbp, rsp
   mov rax, -16; except changes rax is 16 bytes block counter
   pxor xmm0, xmm0;0 is ends
.not_found:
   add rax, 16; except changes ZF after call pcmpistri
   pcmpistri xmm0, [rdi+rax], 00001000b;equal to symbol
   ;pcmpistri: result in rcx, if no result, rcx = 16(blocksize in bytes)
   ;imm8:0-reserved, 0-lowest index, 00 positive orientation,
   ;10 find any from arg2, 00 - packed unsigned bytes
   jnz .not_found; 0 finded? zf if finded /0 in current block
   add rax, rcx; index of ends in rcx
   inc rax; correct index after begin of string
   leave
   ret
