;cmdline.asm
extern printf
section .data
    msg db "The command and arguments: ",10,0
    fmt db "%s",10,0
section .bss
section .text
    global main
main:
    push rbp
    mov rbp, rsp
    mov r12, rdi; argc by default(system v)
    mov r13, rsi; argv by default(system v)
    mov rdi, msg
    call printf; print header
    xor r14, r14
.ploop:
    mov rdi, fmt
    mov rsi, qword[r13+r14*8];next argument
                             ;argv+counter*8 bytes(shift in bytes)
    call printf
    inc r14; ++ counter
    cmp r14, r12
    jl .ploop; if counter==count_of_arguments, break
    leave
    ret



