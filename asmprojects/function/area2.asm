;area.asm
extern printf;
section .data
   radius dq 9.0 ; constexpr
   fmt db "The radius is %f",10,0
section .bss
section .text
;area function---------------------------
area:
   section .data
      .pi dq 3.141592654;.name is local variable(with local scope)
   section .text
      push rbp
      mov rbp, rsp
      movsd xmm0, [radius]
      mulsd xmm0, [radius]
      mulsd xmm0, [.pi]
      leave
      ret
; circum function--------------------------------
circum:
   section .data
      .pi dq 3.14
   section .text
   push rbp
   mov rbp, rsp
   movsd xmm0, [radius]
   addsd xmm0, [radius]
   mulsd xmm0, [.pi]
   leave
   ret
; circle function--------------------------
circle:
   section .data
      .fmtarea db "The area is %f",10,0
      .fmt_circum db "The circumference is %f",10,0
   section .text
      push rbp
      mov rbp, rsp
      call area
      mov rdi, .fmtarea
      mov rax, 1
      call printf
      call circum
      mov rdi, .fmt_circum
      mov rax, 1
      call printf
      leave
      ret
; main function-----------------------------
   global main
main:
   push rbp;
   mov rbp, rsp;
   mov rax, 1
   movsd xmm0, [radius]
   mov rdi, fmt
   call printf
   call circle
   leave
   ret


