;macro.asm
extern printf
;-------------------------------------------------
%define double_it(r) sal r, 1; singleline macro
;-------------------------------------------------
%macro print 2; multiline macro with 2 args
   section .data
      %%arg1 db %1,0 ;%% is local variable in macro, without %% - define in ; each paste of macro body
      %%fmtint db "%s %ld",10,0
   section .text
      mov rdi, %%fmtint
      mov rsi, %%arg1
      mov rdx, [%2]
      xor rax, rax
      call printf
%endmacro
;--------------------------------------------------
section .data
   number dq 15
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   print "The number is", number
   mov rax, [number]
   double_it(rax)
   mov qword[number], rax
   print "The number times 2 is", number
   leave
   ret
