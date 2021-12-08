;args.asm
extern printf
section .data
   first db "A",0
   second db "B",0
   third db "C",0
   fourth db "D",0
   fifth db "E",0
   sixth db "F",0
   seventh db "G",0
   eighth db "H",0
   nineth db "I",0
   tenth db "J",0
   fmt db "The string is %s",10,0
   fmt1 db "%s%s%s%s%s%s%s%s%s%s",10,0
   fmt2 db "Pi = %f",10,0
   pi dq 3.14
section .bss
   flist resb 11; char array
section .text
global main
main:
    push rbp;
    mov rbp, rsp;
    mov rdi, flist
    mov rsi, first
    mov rdx, second
    mov rcx, third
    mov r8, fourth
    mov r9, fifth
    push tenth
    push nineth
    push eighth
    push seventh
    push sixth
    call lfunc
    xor rax, rax
    mov rdi, fmt1
    call printf
    xor rax, rax
    mov rdi, fmt
    mov rsi, flist
    call printf
    mov rsp, rbp
    movsd xmm0, [pi]
    mov rax, 1
    mov rdi, fmt2
    call printf
    leave
    ret
;lfunc-------------
lfunc: 
   push rbp
   mov rbp, rsp
   mov al, byte[rsi]
   mov [rdi], al; &result_array[0] in rdi
   mov al, byte[rdx]
   mov [rdi+1], al
   mov al, byte[rcx]
   mov [rdi+2], al
   mov al, byte[r8]
   mov [rdi+3], al
   mov al, byte[r9]
   mov [rdi+4], al
   push rbx; maybe in rbx something important
   xor rbx, rbx
   mov rax, qword[rbp+16]; 8 byte by call, 8 byte by push rbp in prologue
   mov bl, byte[rax]
   mov [rdi+5], bl
   mov rax, qword[rbp+24]
   mov bl, byte[rax]
   mov [rdi+6], bl
   mov rax, qword[rbp+32]
   mov bl, byte[rax]
   mov [rdi+7], bl
   mov rax, qword[rbp+40]
   mov bl, byte[rax]
   mov [rdi+8], bl
   mov rax, qword[rbp+48]
   mov bl, byte[rax]
   mov [rdi+9], bl
   mov bl, 0
   mov [rdi+10], bl
   pop rbx
   mov rsp, rbp
   pop rbp
   ret





