;hello.asm
section .data
   msg db "hello, world",0xa, 0 ;name type val 0xa='/n' 0='/0'
section .bss
section .text
   global main

main:
   mov rax, 1 ; 1 = write syscall
   mov rdi, 1 ; 1 = to stdout
   mov rsi, msg ; string to display in rsi
   mov rdx, 13 ; length without '/0'
   syscall ; display string
   mov rax, 1
   mov rdi, 1
   mov rax, 60 ; 60 = exit syscall
   mov rdi, 0 ; exit code success
   syscall ; quit
