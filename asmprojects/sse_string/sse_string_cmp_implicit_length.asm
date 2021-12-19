;sse_string_cmp_implicit_length.asm
extern printf
section .data
   string1 db "The quick brown fox jumps over the lazy"
           db " river",10,0
   string2 db "The quick brown fox jumps over the lazy"
           db " river",10,0
   string3 db "The quick brown fox jumps over the lazy"
           db " dog",10,0
   fmt1    db "strings 1 and 2 are equal",10,0
   fmt11   db "strings 1 and 2 differ at position %i",10,0
   fmt2    db "strings 2 and 3 are equal",10,0
   fmt22   db "strings 2 and 3 differ at position %i",10,0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;print strings
   mov rdi, string1
   xor rax, rax
   call printf
   mov rdi, string2
   xor rax, rax
   call printf
   mov rdi, string3
   xor rax, rax
   call printf
;compare 1 and 2
   mov rdi, string1
   mov rsi, string2
   call pstrcmp
   mov rdi, fmt1
   cmp rax, 0
   je eql1
   mov rdi, fmt11
eql1:
   mov rsi, rax
   xor rax, rax
   call printf
;compare 2 and 3
   mov rdi, string2
   mov rsi, string3
   call pstrcmp
   mov rdi, fmt2
   cmp rax, 0
   je eql2
   mov rdi, fmt22
eql2:
   mov rsi, rax
   xor rax, rax
   call printf
   leave
   ret
;----------------------------
pstrcmp:
   push rbp
   mov rbp, rsp
   xor rax, rax
   xor rbx, rbx
.loop:
   movdqu xmm1, [rdi+rbx]
   pcmpistri xmm1, [rsi+rbx], 0x18;00011000equal each|negative orientation
   ; result in ecx
   jc .differ
   jz .equal
   add rbx, 16
   jmp .loop;continue
.differ:
   mov rax, rbx
   add rax, rcx;pos of first not_equal symbol
   inc rax;starting from 1 
.equal:
   leave
   ret
