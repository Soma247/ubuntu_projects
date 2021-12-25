;transposition4x4.asm
extern printf
extern printm4x4
section .data
   fmt0 db "4x4 double precision floating point matrix transpose",10,0
   fmt1 db 10,"This is the matrix:",10,0
   fmt2 db 10,"This is the transpose(unpack):",10,0
   fmt3 db 10,"This is the transpose(shuffle):",10,0
align 32
   matrix dq  1.,  2.,  3.,  4.
          dq  5.,  6.,  7.,  8.
          dq  9., 10., 11., 12.
          dq 13., 14., 15., 16.
section .bss
alignb 32
   transpose resd 16
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   mov rdi, fmt1
   call printf
   mov rsi, matrix
   call printm4x4
   mov rdi, matrix
   mov rsi, transpose
   call transpose_unpacked_4x4
   ;print result
   mov rdi, fmt2
   xor rax, rax
   call printf
   mov rsi, transpose
   call printm4x4
   ;calculate transpose with shuffle
   mov rdi, matrix
   mov rsi, transpose
   call transpose_shuffle_4x4
   ;print result
   mov rdi, fmt3
   xor rax, rax
   call printf
   mov rsi, transpose
   call printm4x4
   leave
   ret
;--------------------------
transpose_unpacked_4x4:
   push rbp
   mov rbp, rsp
   vmovapd ymm0, [rdi];row#0 1  2  3  4 
   vmovapd ymm1, [rdi+32];   5  6  7  8
   vmovapd ymm2, [rdi+64];   9 10 11 12
   vmovapd ymm3, [rdi+96];  13 14 15 16
   ;unpack
   vunpcklpd ymm12, ymm0, ymm1; 1  5  3  7
   ; unpack first lower qword from #2 and #3, save in #1, than unpack second
   ;lower...
   ;ymm:h1 l1 h0 l0 qwords
   ;ymm0 4 3 2 1 packed(forward ordered, physical is (2 1) (4 3))
   ;ymm1 8 7 6 5 packed
   ;ymm12 7(l11) 3(l01) 5(l10) 1(l00)
   
   vunpckhpd ymm13, ymm0, ymm1; 2  6  4  8
   ;unpack first and second highers qwords from #2 and #3
   vunpcklpd ymm14, ymm2, ymm3; 9 13 11 15
   vunpckhpd ymm15, ymm2, ymm3;10 14 12 16

   vmovapd [rsi], ymm12
   vmovapd [rsi+32], ymm13
   vmovapd [rsi+64], ymm14
   vmovapd [rsi+96], ymm15

   ;permutation
   vperm2f128 ymm0, ymm12, ymm14, 00100000b; 1  5   9 13
   ;permutate two 128 bit values in ymm, mask uses forward order(4 3 2 1):
   ;Permute 128-bit floating-point fields in ymm2 and ymm3/mem using controls
      ;from imm8 and store result in ymm1.
   ;VPERM2F128 ymm#1, ymm#2, ymm#3/m256, imm8;
   ;Imm8[1:0] select the source for the first destination 128-bit field,
      ;01 - write higher value from #2 to #1
   ;imm8[5:4] select the source for the second destination field.
      ;11 - write higher value from #2 to #1
   ;If imm8[3] is set, the low 128-bit field is zeroed.
   ;If imm8[7] is set, the high 128-bit field is zeroed.
   vperm2f128 ymm1, ymm13, ymm15, 00100000b; 2  6  10 14
   vperm2f128 ymm2, ymm12, ymm14, 00110001b; 3  7  11 15
   vperm2f128 ymm3, ymm13, ymm15, 00110001b; 4  8  12 16
   ;write in memory
   vmovapd [rsi], ymm0
   vmovapd [rsi+32], ymm1
   vmovapd [rsi+64], ymm2
   vmovapd [rsi+96], ymm3
   leave
   ret
;--------------------------------------
transpose_shuffle_4x4:
   push rbp
   mov rbp, rsp
   vmovapd ymm0, [rdi];row#0 1  2  3  4
   vmovapd ymm1, [rdi+32];   5  6  7  8
   vmovapd ymm2, [rdi+64];   9 10 11 12
   vmovapd ymm3, [rdi+96];  13 14 15 16
   ;shuffle
   vshufpd ymm12, ymm0, ymm1, 0000b; 1  5  3  7
   vshufpd ymm13, ymm0, ymm1, 1111b; 2  6  4  8
   vshufpd ymm14, ymm2, ymm3, 0000b; 9 13 11 15
   vshufpd ymm15, ymm2, ymm3, 1111b;10 14 12 16
   ;VSHUFPD ymm#1, ymm#2, ymm#3/m256, imm8
   ;Selects a double-precision floating-point value of an input pair using a 
   ;bit control and move to a designated element of the destination operand.
   ;The low-to-high order of double-precision element of the destination
   ;operand is interleaved between the first source operand and the second
   ;source operand at the granularity of input pair of 128 bits.
   ;Each bit in the imm8 byte, starting from bit 0, is the select control
   ;of the corresponding element of the destination to received the shuffled
   ;result of an input pair.
   ;Shuffle four pairs of double-precision floating-point values
         ;from ymm2 and ymm3/m256 using imm8 to select from each
         ;pair, interleaved result is stored in xmm1.
         ;mask:3210
         ;mask[3]:0-lower from #3, 1-higher from #3
         ;mask[2]:0-lower from #2, 1-higher from #2
         ;mask[1]:0-lower from #3, 1-higher from #3
         ;mask[0]:0-lower from #2, 1-higher from #2
         ;#2 a3 a2 a1 a0
         ;#3 b3 b2 b1 b0
         ;mask 0000
         ;result b2 a2 b0 a0
         ;mask 1111
         ;result b3 a3 b1 a1
         ;mask 0110
         ;result b2 a3 b1 a0
         ;mask 0011
         ;result b2 a2 b1 a1
   ;permutation
   vperm2f128 ymm0, ymm12, ymm14, 00100000b; 1  5   9  13
   vperm2f128 ymm1, ymm13, ymm15, 00100000b; 2  6  10  14
   vperm2f128 ymm2, ymm12, ymm14, 00110001b; 3  7  11  15
   vperm2f128 ymm3, ymm13, ymm15, 00110001b; 4  8  12  16
   ;write in memory
   vmovapd [rsi], ymm0
   vmovapd [rsi+32], ymm1
   vmovapd [rsi+64], ymm2
   vmovapd [rsi+96], ymm3
   leave
   ret
