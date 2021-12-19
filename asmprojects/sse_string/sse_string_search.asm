;sse_string_length.asm
;WARNING may be a SEGFAULT, functions get blocks of 16 bytes, need simple;
;search in last block or 15bytes of '/0' barriers after strings!!!
extern printf
section .data
;template      0123456789abcdef0123456789abcdef0123456789abc  d
;template      123456789012345678901234567890123456789012345  6
   string1 db "The quick brown fox jumps over the lazy river",0
   string2 db "e",0
   fmt1    db "This is our string: %s",10,0
   fmt2    db "The first '%s' is at position %d.",10,0
   fmt3    db "The last '%s' is at position %d.",10,0
   fmt4    db "The character '%s' didn't show up.",10,0
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
; first encounter
   mov rdi, string1
   mov rsi, string2
   call pstrscan_f
   cmp rax, 0
   je no_show;if returned 0, no encounters in string
   mov rdi, fmt2
   mov rsi, string2
   mov rdx, rax
   xor rax, rax
   call printf
; last encounter
   mov rdi, string1
   mov rsi, string2
   call pstrscan_l
   mov rdi, fmt3
   mov rsi, string2
   mov rdx, rax
   xor rax, rax
   call printf
   jmp exit
no_show:
   mov rdi, fmt4
   mov rsi, string2
   xor rax, rax
   call printf
exit:
   leave
   ret
;---------------------
pstrscan_f:
   push rbp
   mov rbp, rsp
   xor rax, rax
   pxor xmm0, xmm0
   pinsrb xmm0, [rsi],00000000b; set lower byte from [rsi](byte to search
         ;to lower byte of xmm0
.block_loop:
   pcmpistri xmm0, [rdi+rax], 00000000b; search arg(find any) in 16byte
   ;block, counted by rax, result in rcx, if no result, rcx is 0x10(16)
   jc .found;check CF flag, signalized of equality
   jz .not_found; if ZF, this is last block(/0 byte is finded)
   add rax, 16
   jmp .block_loop
.found:
   add rax, rcx;symbol pos in rcx(in current block), rax is bytes counter
               ;of begin of current block
   inc rax;position starting from "1"
   jmp .exit
.not_found:
   xor rax, rax
.exit:
   leave
   ret
;---------------------------------------
pstrscan_l:
   push rbp
   mov rbp, rsp
   push rbx;callee saved
   push r12;callee saved
   xor rax, rax
   pxor xmm0, xmm0
   pinsrb xmm0, [rsi], 00000000b
   xor r12, r12
.block_loop:
   pcmpistri xmm0, [rdi+rax], 01000000b; search argument in 16byte blocks, 
   ;counted by rax
   setz bl; set byte 01h to bl, if ZF=1(ends founded)
   jc .found
   jz .done
   add rax, 16;next block
   jmp .block_loop
.found:
   mov r12, rax
   add r12, rcx
   inc r12;last saved encounting pos saved in r12
   cmp bl, 1;ends founded in last block?
   je .done; founded, break
   add rax, 16; pointer to next block
   jmp .block_loop; continue
.done:
   mov rax, r12
   pop r12
   pop rbx
   leave
   ret
