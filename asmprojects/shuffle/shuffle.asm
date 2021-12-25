;shuffle.asm
extern printf
section .data
   fmt0  db "These are numbers in memory: ",10,0
   fmt00 db "This is xmm0: ",10,0
   fmt1  db "%d ",0
   fmt2  db "Shuffle-broadcast double word %i:",10,0
   fmt3  db "%d %d %d %d",10,0
   fmt4  db "Shuffle-reverse double words:",10,0
   fmt5  db "Shuffle-reverse packed bytes in xmm0:",10,0
   fmt6  db "Shuffle-rotate left:",10,0
   fmt7  db "Shuffle-rotate right:",10,0
   fmt8  db "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",10,0
   fmt9  db "Packed bytes in xmm0:",10,0
   char  db "abcdefghijklmnop"
   NL    db 10,0
   bytereverse db 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
                ;bytemask for shuffle bytes:
                ;#  0  1  2  3  4  5 6 7 8 9 a b c d e f
                ; (byte #15 in 0 pos, 14 in 1 pos, etc)
   number1 dd 1
   number2 dd 2
   number3 dd 3
   number4 dd 4
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   sub rsp, 32;reserve 4 stack frames for saving original and changed xmm0
;shuffling dwords
;reversed print
   mov rdi, fmt0
   call printf
   mov rdi, fmt1
   mov rsi, [number4]
   xor rax, rax
   call printf
   mov rdi, fmt1
   mov rsi, [number3]
   xor rax, rax
   call printf
   mov rdi, fmt1
   mov rsi, [number2]
   xor rax, rax
   call printf
   mov rdi, fmt1
   mov rsi, [number1]
   xor rax, rax
   call printf
   mov rdi, NL
   call printf
;fill xmm0
   pxor xmm0, xmm0
   pinsrd xmm0, dword[number1],0; load number1 as #0 dword in pack of xmm0
   pinsrd xmm0, dword[number2],1
   pinsrd xmm0, dword[number3],2
   pinsrd xmm0, dword[number4],3
   movdqu [rbp-16], xmm0;save xmm0 in first dedicated frame(rsp-
   mov rdi, fmt00
   call printf;print header
   movdqu xmm0, [rbp-16];restore xmm0 after printf
   call print_xmm0d;print xmm0
   movdqu xmm0, [rbp-16];restore xmm0
;random shuffle
;for lower dword(index 0)
   movdqu xmm0, [rbp-16];restore xmm0
   pshufd xmm0, xmm0, 00000000b;shuffle mask:
                                 ;00 00 00 00 dword #0 to all
   mov rdi, fmt2
   xor rsi, rsi;print header for #0
   movdqu [rbp-32], xmm0; save xmm0 in 2 higher reserved frames(lower adr)
   call printf
   movdqu xmm0, [rbp-32];restore resulted xmm0
   call print_xmm0d; print xmm0
;random shuffle dword #1
   movdqu xmm0, [rbp-16];restore default xmm0
   pshufd xmm0, xmm0, 01010101b;shuffle mask: 01 01 01 01, dword#1 to all
   mov rdi, fmt2
   mov rsi, 1; for #1
   movdqu [rbp-32], xmm0;save xmm0
   call printf;print header
   movdqu xmm0, [rbp-32];restore xmm0
   call print_xmm0d; print xmm0
;random shuffle dword #2
   movdqu xmm0, [rbp-16];restore default xmm0
   pshufd xmm0, xmm0, 10101010b;shuffle mask 
                                 ;10 10 10 10, dword 2(10) to all
   mov rdi, fmt2
   mov rsi, 2
   movdqu [rbp-32], xmm0
   call printf
   movdqu xmm0, [rbp-32]
   call print_xmm0d
;random shuffle dword #3
   movdqu xmm0, [rbp-16]
   pshufd xmm0, xmm0, 11111111b
   mov rdi, fmt2
   mov rsi, 3
   movdqu [rbp-32], xmm0
   call printf
   movdqu xmm0, [rbp-32]
   call print_xmm0d
;reversed dwords
   movdqu xmm0, [rbp-16]
   pshufd xmm0, xmm0, 00011011b; 0 1 2 3 (3 2 1 0 in default indexes)
   mov rdi, fmt4
   movdqu [rbp-32], xmm0
   call printf
   movdqu xmm0, [rbp-32]
   call print_xmm0d
;rotate left
   movdqu xmm0, [rbp-16]
   pshufd xmm0, xmm0, 10010011b;2 1 0 3
   mov rdi, fmt6
   movdqu [rbp-32], xmm0
   call printf
   movdqu xmm0, [rbp-32]
   call print_xmm0d
;rotate right
   movdqu xmm0, [rbp-16]
   pshufd xmm0, xmm0, 00111001b;0 3 2 1
   mov rdi, fmt7
   movdqu [rbp-32], xmm0
   call printf
   movdqu xmm0, [rbp-32]
   call print_xmm0d
;shuffle bytes
   mov rdi, fmt9
   call printf
   movdqu xmm0, [char]; load bytes(16) from memory at *char to xmm0
   movdqu [rbp-32], xmm0; save xmm0
   call print_xmm0b
   movdqu xmm0, [rbp-32]
   movdqu xmm1, [bytereverse];load mask for shuffle
   pshufb xmm0, xmm1;shuffle bytepack in xmm0 by mask in xmm1
   mov rdi, fmt5
   movdqu [rbp-32], xmm0;save shuffled pack
   call printf
   movdqu xmm0, [rbp-32];restore pack
   call print_xmm0b
   leave
   ret
;---------------------------------
print_xmm0d:
   push rbp
   mov rbp, rsp
   mov rdi, fmt3
   xor rax, rax
   pextrd esi, xmm0, 3;extract dwords(4 byte) to 32bit registers
   pextrd edx, xmm0, 2
   pextrd ecx, xmm0, 1
   pextrd r8d, xmm0, 0
   call printf
   leave
   ret
;----------------------------------------
print_xmm0b:
   push rbp
   mov rbp, rsp
   mov rdi, fmt8;count in fmt
   pextrb esi, xmm0, 0
   pextrb edx, xmm0, 1
   pextrb ecx, xmm0, 2
   pextrb r8d, xmm0, 3
   pextrb r9d, xmm0, 4
   pextrb eax, xmm0, 15
   ;than stack
   push rax
   pextrb eax, xmm0, 14
   push rax
   pextrb eax, xmm0, 13
   push rax
   pextrb eax, xmm0, 12
   push rax
   pextrb eax, xmm0, 11
   push rax
   pextrb eax, xmm0, 10
   push rax
   pextrb eax, xmm0, 9
   push rax
   pextrb eax, xmm0, 8
   push rax
   pextrb eax, xmm0, 7
   push rax
   pextrb eax, xmm0, 6
   push rax
   pextrb eax, xmm0, 5
   push rax
   xor rax, rax
   call printf
   leave
   ret

