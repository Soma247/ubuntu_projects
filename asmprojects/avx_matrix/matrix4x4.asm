;matrix4x4.asm
extern printf
extern printm4x4
section .data
   fmt0 db 10, "4x4 double precision floating point matrices",10,0
   fmt1 db 10, "Matrix A:",10,0
   fmt2 db 10, "Matrix B:",10,0
   fmt3 db 10, "Matrix A x Matrix B:",10,0
   fmt4 db 10, "Matrix C:",10,0
   fmt5 db 10, "Inversed matrix C:",10,0
   fmt6 db 10, "Proof: matrix C x inversed matrix C:",10,0
   fmt7 db 10, "Matrix S:",10,0
   fmt8 db 10, "inversed matrix S:",10,0
   fmt9 db 10, "Proof: matrix S x inversed matrix S:",10,0
   fmt10 db 10, "Singular matrix detected",10,0
align 32; 32 bytes aligning(ymm is 256 register)
   MatrixA  dq  1.,  3.,  5.,  7.
            dq  9., 11., 13., 15.
            dq 17., 19., 21., 23.
            dq 25., 27., 29., 31.

   MatrixB  dq  2.,  4.,  6.,  8.
            dq 10., 12., 14., 16.
            dq 18., 20., 22., 24.
            dq 26., 28., 30., 32.

   MatrixC  dq 2., 11., 21., 37.
            dq 3., 13., 23., 41.
            dq 5., 17., 29., 43.
            dq 7., 19., 31., 47.

   MatrixS  dq  1.,  2.,  3.,  4.
            dq  5.,  6.,  7.,  8.
            dq  9., 10., 11., 12.
            dq 13., 14., 15., 16.
section .bss
alignb 32
   product  resq 16
   inversed resq 16
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   mov rdi, fmt0
   call printf;header
;print a
   mov rdi, fmt1
   call printf
   mov rsi, MatrixA
   call printm4x4
;print b 
   mov rdi, fmt2
   call printf
   mov rsi, MatrixB
   call printm4x4
;product axb
   mov rdi, MatrixA
   mov rsi, MatrixB
   mov rdx, product
   call multi4x4
;print product
   mov rdi, fmt3
   call printf
   mov rsi, product
   call printm4x4
;print c
   mov rdi, fmt4
   call printf
   mov rsi, MatrixC
   call printm4x4
;inversion
   mov rdi, MatrixC
   mov rsi, inversed
   call inverse4x4
   cmp rax, 1
   je singular
;print inversed
   mov rdi, fmt5
   call printf
   mov rsi, inversed
   call printm4x4
;proof c
   mov rsi, MatrixC
   mov rdi, inversed
   mov rdx, product
   call multi4x4
;print proof
   mov rdi, fmt6
   call printf
   mov rsi, product
   call printm4x4
; singular matrix
; print s
   mov rdi, fmt7
   call printf
   mov rsi, MatrixS
   call printm4x4
;inverse s
   mov rdi, MatrixS
   mov rsi, inversed
   call inverse4x4
   cmp rax, 1
   je singular
;print inversed
   mov rsi, fmt8
   call printf
   mov rsi, inversed
   call printm4x4
;proof s x inversed
   mov rsi, MatrixS
   mov rdi, inversed
   mov rdx, product
   call multi4x4
; print proof
   mov rdi, fmt9
   call printf
   mov rsi, product
   call printm4x4
   jmp exit
singular:
   mov rdi, fmt10
   call printf
exit:
   leave
   ret
;--------------------------
inverse4x4:
section .data
align 32
   .identity dq 1., 0., 0., 0.
             dq 0., 1., 0., 0.
             dq 0., 0., 1., 0.
             dq 0., 0., 0., 1.
   .minus_mask dq 8000000000000000h
   .size       dq 4
   .one        dq 1.0
   .two        dq 2.0
   .three      dq 3.0
   .four       dq 4.0
section .bss
alignb 32
   .matrix1 resq 16;intermediate matrices
   .matrix2 resq 16;pow 2
   .matrix3 resq 16;pow3
   .matrix4 resq 16
   .matrixI resq 16
   .mxcsr   resd 1; using in divide by 0 check
section .text
   push rbp
   mov rbp, rsp
   push rsi;save ard inversed matrix
   vzeroall;clearr all ymm's
;source matrix in rdi, calculate .matrix2
   mov rsi, rdi
   mov rdx, .matrix2
   push rdi
   call multi4x4
   pop rdi
;calculate .matrix3
   mov rsi, .matrix2
   mov rdx, .matrix3
   push rdi
   call multi4x4
   pop rdi
;calculate .matrix4
   mov rsi, .matrix3
   mov rdx, .matrix4
   push rdi
   call multi4x4
   pop rdi
;calculate trace1
   mov rsi, [.size]
   call vtrace
   movsd xmm8, xmm0; trace1 in xmm8
;trace2
   push rdi;save adr source matrix
   mov rdi, .matrix2
   mov rsi, [.size]
   call vtrace
   movsd xmm9, xmm0; trace2 in xmm9
;trace3
   mov rdi, .matrix3
   mov rsi, [.size]
   call vtrace
   movsd xmm10, xmm0
;trace4
   mov rdi, .matrix4
   mov rsi, [.size]
   call vtrace
   movsd xmm11, xmm0
;calculate coefficients (1/-p4)(A^3 + p1*A^2 + p3*I)=A^-1
;p1=-s1
   vxorpd xmm12, xmm8, [.minus_mask]; p1 in xmm12
;p2=-1/2*(p1*s1+s2)
   movsd xmm13, xmm12; copy p1 to xmm13
   vfmadd213sd xmm13, xmm8, xmm9; xmm13=xmm13*xmm8+xmm9(code is:#2*#1+#3)
   vxorpd xmm13, xmm13, [.minus_mask]
   divsd xmm13, [.two]; p2 in xmm13
;p3 = -1/3*(p2*s1+p1*s2+s3)
   movsd xmm14, xmm12;pi to xmm14
   vfmadd213sd xmm14, xmm9, xmm10;xmm14=xmm14*xmm9+xmm10
   vfmadd231sd xmm14, xmm13, xmm8;xmm14=xmm13*xmm8+xmm14
   vxorpd xmm14, xmm14, [.minus_mask]
   divsd xmm14, [.three];p3 in xmm14
;p4 =-1/4(p3*s1+p2*s2+p1*s3+s4)
   movsd xmm15, xmm12
   vfmadd213sd xmm15, xmm10, xmm11;xmm15=xmm15*xmm10+xmm11
   vfmadd231sd xmm15, xmm13, xmm9
   vfmadd231sd xmm15, xmm14, xmm8
   vxorpd xmm15, xmm15, [.minus_mask]
   divsd xmm15, [.four];p4 in xmm15
;multiplication with the coefficients
   mov rcx, [.size]
   xor rax, rax
   vbroadcastsd ymm1, xmm12; p1
   vbroadcastsd ymm2, xmm13; p2
   vbroadcastsd ymm3, xmm14; p3
   pop rdi; restore source matrix addr
   .loop1:
      vmovapd ymm0, [rdi+rax]
      vmulpd ymm0, ymm0, ymm2
      vmovapd [.matrix1+rax], ymm0
      vmovapd ymm0, [.matrix2+rax]
      vmulpd ymm0, ymm0, ymm1
      vmovapd [.matrix2+rax], ymm0
      vmovapd ymm0, [.identity+rax]
      vmulpd ymm0, ymm0, ymm3
      vmovapd [.matrixI+rax], ymm0
      add rax, 32
      loop .loop1
;summ and *-1/p4
   mov rcx, [.size]
   xor rax, rax
   movsd xmm0, [.one]
   vdivsd xmm0, xmm0, xmm15
;divide by zero check
   stmxcsr [.mxcsr];store
   and dword[.mxcsr], 4; check flag 
   jnz .singular; divided by 0
   pop rsi
   vxorpd xmm0, xmm0, [.minus_mask];-1/p4
   vbroadcastsd ymm2, xmm0
;cycle by rows
   .loop2:
      vmovapd ymm0, [.matrix1+rax]
      vaddpd ymm0, ymm0, [.matrix2+rax]
      vaddpd ymm0, ymm0, [.matrix3+rax]
      vaddpd ymm0, ymm0, [.matrixI+rax]
      vmulpd ymm0, ymm0, ymm2;row*-1/p4
      vmovapd [rsi+rax], ymm0
      add rax, 32
      loop .loop2
   xor rax, rax
   leave
   ret

.singular:
   mov rax, 1;singular matrix, return error 1
   leave
   ret
;----------------------------
vtrace:
  push rbp
  mov rbp, rsp
  vmovapd  ymm0, [rdi];4 rows of matrix
  vmovapd  ymm1, [rdi+32]
  vmovapd  ymm2, [rdi+64]
  vmovapd  ymm3, [rdi+96]
  vblendpd ymm0, ymm0, ymm1, 0010b;trace, from $2 and $3 to $1, mask is $4
  ;ymm0:a3 a2 a1 a0
  ;ymm1:b3 b2 b1 b0; mask 0010
  ;ymm0_result:a3 a2 b1 a0
  vblendpd ymm0, ymm0, ymm2, 0100b;
  ;ymm2 c3 c2 c1 c0, mask 0100
  ;ymm0-> a3 c2 b1 a0
  vblendpd ymm0, ymm0, ymm3, 1000b
  ;ymm0->d3 c2 b1 a0
  ;result is m33 m22 m11 m00(main diagonal of matrix)
  vhaddpd  ymm0, ymm0, ymm0;horizontal add packed double
  ;result a33+a22 a33+a22 a11+a00 a11+a00
  vpermpd  ymm0, ymm0, 00100111b;permutation:
  ;00 10 01 11
  ;(0)(2)(1)(3)
  ;a11+a00 a33+a22 a11+a00 a33+a22
  haddpd   xmm0, xmm0
  ;xmm0: a11+a00+a33+a22(summ of diag elements) "cause i can!!1"
  leave
  ret
;------------------------------------
multi4x4:
   push rbp
   mov rbp, rsp
   xor rax, rax
   mov rcx, 4
   vzeroall
   .loop:
      vmovapd ymm0, [rsi];4 double floating points from rsi(B) to ymm0
      vbroadcastsd ymm1, [rdi+rax];column from rdi(A) to ymm1,
                                 ;rax is column counter 
      vfmadd231pd ymm12, ymm1, ymm0
      vbroadcastsd ymm1, [rdi+32+rax];write from memory to each of 4 qwords
                                       ;in ymm1
      vfmadd231pd ymm13, ymm1, ymm0;ymm13+=ymm1*ymm0
      vbroadcastsd ymm1, [rdi+64+rax]
      vfmadd231pd ymm14, ymm1, ymm0
      vbroadcastsd ymm1, [rdi+96+rax]
      vfmadd231pd ymm15, ymm1, ymm0
      add rax, 8; sizeof one floating point value is 8 bytes(64 bits)
      add rsi, 32; 32 bytes in row, 256 bits(4 elements)
      loop .loop
   ;result to mem
   vmovapd [rdx], ymm12
   vmovapd [rdx+32], ymm13
   vmovapd [rdx+64], ymm14
   vmovapd [rdx+96], ymm15
   xor rax, rax;return zero
   leave
   ret
