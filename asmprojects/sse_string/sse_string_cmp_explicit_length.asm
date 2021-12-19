;sse_string_cmp_explicit_length.asm
extern printf
section .data
   string1 db "The quick brown fox jumps over the lazy"
           db " river"
   len1    equ $-string1
   string2 db "The quick brown fox jumps over the lazy"
           db " river"
   len2    equ $-string2
   dummy   db "confuse the world"
   string3 db "The quick brown fox jumps over the lazy"
           db " dog"
   len3    equ $-string3
   fmt1    db "strings 1 and 2 are equal",10,0
   fmt11   db "strings 1 and 2 differ at position %i",10,0
   fmt2    db "strings 2 and 3 are equal",10,0
   fmt22   db "strings 2 and 3 differ at position %i",10,0
section .bss
   buffer resb 64
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;compare 1 and 2
   mov rdi, string1
   mov rsi, string2
   mov rdx, len1
   mov rcx, len2
   call pstrcmp
   push rax;save result in stack
; print 1, 2 and result of compare
   mov rsi, string1
   mov rdi, buffer
   mov rcx, len1
   rep movsb; copy str1 to rdi without ends
   mov byte[rdi],10;add nl
   inc rdi
   mov byte[rdi], 0;and ends to buffer tail
;print
   mov rdi, buffer
   xor rax, rax
   call printf
;str2
   mov rsi, string2
   mov rdi, buffer
   mov rcx, len2
   rep movsb
   mov byte[rdi],10
   inc rdi
   mov byte[rdi],0
   mov rdi, buffer
   xor rax, rax
   call printf
; print cmpresult
   pop rax;restore result of cmp
   mov rdi, fmt1
   cmp rax, 0
   je eql1
   mov rdi, fmt11
eql1:
   mov rsi, rax
   xor rax, rax
   call printf
;cmp 2 and 3
   mov rdi, string2
   mov rsi, string3
   mov rdx, len2
   mov rcx, len3
   call pstrcmp
   push rax
;print str3 and result
   mov rsi, string3
   mov rdi, buffer
   mov rcx, len3
   rep movsb
   mov byte[rdi],10
   inc rdi
   mov byte[rdi],0
   mov rdi, buffer
   xor rax, rax
   call printf
   pop rax
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
;--------------------------------
pstrcmp:
   push rbp
   mov rbp, rsp
   xor rbx, rbx
   mov rax, rdx; len 1st arg
   mov rdx, rcx; len 2nd arg
   xor rcx, rcx; index
.loop:
   movdqu xmm1, [rdi+rbx]; ptr to first char of 16 byte current block
   pcmpestri xmm1, [rsi+rbx], 0x18;00011000 
                              ;equal each|negative orientation ires2
   jc .differ; CF=0, if intres2==0, else =1
   jz .equal; ZF=1, if abs[edx]<16(8)
   add rbx, 16
   sub rax, 16
   sub rdx, 16
   jmp .loop
.differ:
   mov rax, rbx
   add rax, rcx; position in rcx, block beg pos in rax
   inc rax; starting from 1
   jmp .exit
.equal:
   xor rax, rax
.exit:
   leave
   ret
