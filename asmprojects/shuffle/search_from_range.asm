;search_from_range.asm
extern printf
extern print_mask
extern pstrlen
extern reverse_mm0
section .data
    string1 db "eeAecdkkFijlmeoZabcefgeKlkmeDadfdsafadfaseeE",10,0
    startrange db "A",10,0
    stoprange db "Z",10,0
    NL db 10,0
    fmt db "Find the uppercase letters in:",10,0
    fmt_oc db "I found %ld uppercase letters",10,0
section bss
section text
    global main
main:
    push rbp
    mov rbp, rsp
    mov rdi, fmt
    xor rax, rax
    call printf
    mov rdi, string1
    xor rax, rax
    call printf
;search
    mov rdi, string1
    mov rsi, startrange
    mov rdx, stoprange
    call prangesrch
;print results
    mov rdi, fmt_oc
    mov rsi, rax
    xor rax, rax
    call printf
    leave
    ret
    
prangesrch:
    push rbp
    mov rbp, rsp
    xor r12, r12
    xor rcx, rcx
    xor rbx, rbx
    mov rax, -16
    pxor xmm1, xmm1
    pinsrb xmm1,  byte[rsi], 0
    pinsrb xmm1,  byte[rdx], 1
    .loop:
        add rax, 16
        mov rsi, 16
        movdqu xmm2, [rdi+rbx]
        pcmpistrm xmm1, xmm2, 01000100b;each from eqal range, return mask in xmm0
        setz cl
        cmp cl, 0
        je .print
        add rdi, rbx
        push rcx
        call pstrlen
        pop rcx
        dec rax
        mov rsi, rax
    .print:
        call print_mask
        popcnt r13d, r13d
        add r12d, r13d
        or cl, cl
        jnz .exit
        add rbx, 16
        jmp .loop
 .exit:
    mov rdi, NL
    call printf
    mov rax, r12
    leave
    ret
