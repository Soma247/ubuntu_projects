;shiftbits2.asm
extern printf
extern printbits
section .data
   msgn1 db "Number 1",10,0
   msgn2 db "Number 2",10,0
   msg1  db "SHL 2 = correctly mul by 4",124,0
   msg2  db "SHR 2 = wrong divide by 4",124,0
   msg3  db "SAL 2 = correctly mul by 4",124,0
   msg4  db "SAR 2 = correctly divide by 4",124,0
   msg5  db "SHR 2 = correctly divide by 4",124,0
   number1 dq 8
   number2 dq -8
   result  dq 0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;shl positive
   mov rsi, msg1
   call printmsg
   mov rsi, [number1]
   call printbr; print number
   mov rax, [number1]
   shl rax, 2; logical shift left, mul by 4
   mov rsi, rax
   call printres
;shl negative
   mov rsi, msg1
   call printmsg
   mov rsi, [number2]
   call printbr
   mov rax, [number2]
   shl rax, 2; logical shift left, mul by 4
   mov rsi, rax
   call printres
;sal positive
   mov rsi, msg3
   call printmsg
   mov rsi, [number1]
   call printbr
   mov rax, [number1]
   sal rax, 2; ariphmetic shift, mul by 4
   mov rsi, rax
   call printres
;sal negative
   mov rsi, msg3
   call printmsg
   mov rsi, [number2]
   call printbr
   mov rax, [number2]
   sal rax, 2; ariphpetic shift, mul by 4
   mov rsi, rax
   call printres
;shr positive
   mov rsi, msg5
   call printmsg
   mov rsi, [number1]
   call printbr
   mov rax, [number1]
   shr rax, 2; logical shift right, divide by 4
   mov rsi, rax
   call printres
; shr negative
   mov rsi, msg2
   call printmsg
   mov rsi, [number2]
   call printbr
   mov rax, [number2]
   shr rax, 2; logical shift right, divide by 4
   mov rsi, rax
   call printres
;sar positive
   mov rsi, msg4
   call printmsg
   mov rsi, [number1]
   call printbr
   mov rax, [number1]
   sar rax, 2; ariphmetical shift right, divide by 4
   mov rsi, rax
   call printres
;sar negative
   mov rsi, msg4
   call printmsg
   mov rsi, [number2]
   call printbr
   mov rax, [number2]
   sar rax, 2; ariphmetical shift right, divide by 4
   mov rsi, rax
   call printres
;exit
   mov rsp, rbp
   pop rbp
   ret
;---------------------------
printmsg:
   section .data
      .fmtstr db "%s",0
   section .text 
      mov rdi, .fmtstr
      xor rax, rax
      call printf
   ret
;---------------------------
printbr:
   section .data
      .fmtstr db "The original number is %lld",10,0
   section .text
      mov rdi, .fmtstr
      xor rax, rax
      call printf
      ret
;---------------------------
printres:
   section .data
      .fmtstr db "The resulting number is %lld",10,0
   section .text
      mov rdi, .fmtstr
      mov rax, 0
      call printf
      ret
