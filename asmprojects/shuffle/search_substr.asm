;search_substr.asm
extern printf
extern print_mask
extern pstrlen
extern reverse_mm0
section .data
    string1 db "a quick pink dinosour jumps over the"
            db "lazy river and the lazy dinosour "
            db "doesn't mind",10,0
    string2 db "dinosour",0
    NL db 10,0
    fmt db "Find the substring '%s' in:",10,0
    fmt_oc db "I found %ld %ss",10,0
section bss
section text
    global main
main:
    push rbp
    mov rbp, rsp
    mov rdi, fmt
    mov rsi, string2
    xor rax, rax
    call printf
    mov rdi, string1
    xor rax, rax
    call printf
;search
    mov rdi, string1
    mov rsi, string2
    call psubstrsearch
;print results
    mov rdi, fmt_oc
    mov rsi, rax
    mov rdx, string2
    xor rax, rax
    call printf
    leave
    ret
    
psubstrsearch:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    xor r12, r12
    xor rcx, rcx
    xor rbx, rbx
    mov rax, -16
    pxor xmm1, xmm1
    movdqu xmm1, [rsi];str2
    .loop:
      add rax, 16
      mov rsi, 16; if not ends, print 16 bytes
      movdqu xmm2, [rdi+rbx]
      pcmpistrm xmm1, xmm2, 01001100b; equal ordered char sequence
                                     ; |bytemask in xmm0
      setz cl; ends finded
      cmp cl, 0
      je .print
      ;ends finded, < 16 bytes left, ptr to str in rdi, count of checked
      ;bytes in rbx
      add rdi, rbx;only tail of str
      push rcx;caller saved, used the cl
      call pstrlen;pos of ends in rax
      push rcx
      dec rax; without ends
      mov rsi, rax; bytes left count
   .print:
      call print_mask
      popcnt r13d, r13d;count of bit, that==1
      add r12d, r13d; count of enry
      or cl, cl; ends finded?
      jnz .exit
      add rbx, 16; next block
      jmp .loop
.exit:
   mov rdi, NL
   call printf
   mov rax, r12
   leave
   ret
