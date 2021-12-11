;bts_brr_bt.asm;
extern printf
extern printbits
section .data
   msg1 db "No bits are set:",10,0
   msg2 db 10, "Set bit #4, that is the 5th bit:",10,0
   msg3 db 10, "Set bit #7, that is the 8th bit:",10,0
   msg4 db 10, "Set bit #8, that is the 9th bit:",10,0
   msg5 db 10, "Set bit #61, that is the 62th bit:",10,0
   msg6 db 10, "Clear bit #8, that is the 9th bit:",10,0
   msg7 db 10, "Test bit #61, and display rdi",10,0
   bitflags dq 0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;print header
   mov rdi, msg1
   xor rax, rax
   call printf
;print bitflags
   mov rdi, [bitflags]
   call printbits
;set bit #4
   mov rdi, msg2
   xor rax, rax
   call printf
   bts qword[bitflags], 4; set 4th bit(0,1,...,63 is 64-bitset)
   mov rdi, [bitflags]
   call printbits
;set bit #7
   mov rdi, msg3
   xor rax, rax
   call printf
   bts qword[bitflags], 7
   mov rdi, [bitflags]
   call printbits
;set bit #8
   mov rdi, msg4
   xor rax, rax
   call printf
   bts qword[bitflags], 8
   mov rdi, [bitflags]
   call printbits
;set bit #61
   mov rdi, msg5
   xor rax, rax
   call printf
   bts qword[bitflags],61
   mov rdi, [bitflags]
   call printbits
;unset bit #8
   mov rdi, msg6
   xor rax, rax
   call printf
   btr qword[bitflags], 8
   mov rdi, [bitflags]
   call printbits
;test bit #61 if true, CF=1
   mov rdi, msg7
   xor rax, rax
   call printf
   xor rdi, rdi
   mov rax, 61
   xor rdi, rdi
   bt [bitflags], rax; check bit *(bitflags+rax)
   setc dil; set dil(lower byte of rdi) to 1, if cf is true
   call printbits; print rdi
   mov rsp, rbp
   pop rbp
   ret

