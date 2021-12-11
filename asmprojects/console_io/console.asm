;console.asm
section .data
   msg1 db "Hello, world!",10,0
   len1 equ $-msg1-1
   msg2 db "Your turn(plz, put a-z sequence): ",0
   msg3 db "You answered: ",0
   inputlen equ 10; buffer length
   NL db 0xa
section .bss
   input resb inputlen+1;with '/0'
section .text
   global main
main:
   push rbp
   mov rbp, rsp
   mov rsi, msg1
   mov rdx, len1
   call printsl; print first msg
   mov rdi, msg2
   call prints; print second msg/*
   mov rdi, input; &buf[1] in rdi
   mov rsi, inputlen; bufsize in rsi
   call reads; call reader function
   mov rdi, msg3
   call prints
   mov rdi, input
   call prints
   mov rdi, NL
   call prints
   leave
   ret
;------------------------------------
printsl:
;addr in rsi, len in rdx
   push rbp
   mov rbp, rsp
   mov rax, 1
   mov rdi, 1
   syscall
   leave
   ret
;---------------------------------
prints:
;byte* in rdi
   push rbp
   mov rbp, rsp
   push r12; callie-saved register
;length
   xor rdx, rdx
   mov r12, rdi
.lengthloop:
   cmp byte[r12], 0
   je .endfound
   inc rdx; length++
   inc r12; iterator++
   jmp .lengthloop
.endfound:
   cmp rdx, 0
   je .done; if length==0
   mov rsi, rdi;&str[0]
   mov rdi, 1;stdout
   mov rax, 1; write
   syscall
.done:
   pop r12
   leave
   ret
;-------------------------------------
reads:
section .data
section .bss
   .inputchar resb 1
section .text
   push rbp
   mov rbp, rsp
   push r12; callee saved registers
   push r13
   push r14
   mov r12, rdi;&buff[0]
   mov r13, rsi;maxlen
   xor r14, r14;counter
.readc:
   mov rax, 0; read
   mov rdi, 1; from stdin
   lea rsi, [.inputchar]; charbuf addr
   mov rdx, 1; count of reading bytes = 1
   syscall
   mov al, [.inputchar]; readed char to al
   cmp al, byte[NL]
   je .done; if readed char is newline,goto done
   cmp al, 97
   jl .readc; if readed char < 'a' read next
   cmp al, 122
   jg .readc; if readed char > 'z' read next
   inc r14; counter++
   cmp r14, r13
   ja .readc; if counter > bufsize, read next
   mov byte[r12], al; write readed byte to buffer
   inc r12; increment ptr to buffer
   jmp .readc; read next
.done:
   inc r12; to last buffer character
   mov byte[r12], 0; write '/0'
   pop r14
   pop r13
   pop r12
   leave
   ret
