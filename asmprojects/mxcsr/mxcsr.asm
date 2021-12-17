;mxcsr.asm
extern printf
extern print_hex
section .data
   eleven dq 11.0
   two    dq 2.0
   three  dq 3.0
   ten    dq 10.0
   zero   dq 0.0
   hex      db "0x",0
   fmt1     db 10, "Divide, default mxcsr:",10,0
   fmt2     db 10, "Divide by zero, default mxcsr:",10,0
   fmt4     db 10, "Divide, round up:",10,0
   fmt5     db 10, "Divide, round down:",10,0
   fmt6     db 10, "Divide, truncate:",10,0
   f_div    db "%.1f divided by %.1f is %.16f, in hex: ",0
   f_before db 10, "mxcsr before:",9,0
   f_after  db "mxcsr after:",9,0
;mxcsr values:
   default_mxcsr dd 0001111110000000b
   round_nearest dd 0001111110000000b
   round_down    dd 0011111110000000b
   round_up      dd 0101111110000000b
   truncate      dd 0111111110000000b
section .bss
   mxcsr_before resd 1
   mxcsr_after  resd 1
   xmm          resq 1
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;dividing, default value of mxcsr
   mov rdi, fmt1
   mov rsi, ten
   mov rdx, two
   mov ecx, [default_mxcsr]
   call apply_mxcsr
;dividing with precision lost, default mxcsr
   mov rdi, fmt1
   mov rsi, ten
   mov rdx, three
   mov ecx, [default_mxcsr]
   call apply_mxcsr
;dividing to zero, mxcsr is default
   mov rdi, fmt2
   mov rsi, ten
   mov rdx, zero
   mov ecx, [default_mxcsr]
   call apply_mxcsr
;dividing with precition lost, round up
   mov rdi, fmt4
   mov rsi, ten
   mov rdx, three
   mov ecx, [round_up]
   call apply_mxcsr
;dividing with precition lost, round down
   mov rdi, fmt5
   mov rsi, ten
   mov rdx, three
   mov ecx, [round_down]
   call apply_mxcsr
;dividing with precition lost, truncate result bits
   mov rdi, fmt6
   mov rsi, ten
   mov rdx, three
   mov ecx, [truncate]
   call apply_mxcsr
;dividing, default value of mxcsr
   mov rdi, fmt1
   mov rsi, eleven
   mov rdx, three
   mov ecx, [default_mxcsr]
   call apply_mxcsr
;dividing with precition lost, round up
   mov rdi, fmt4
   mov rsi, eleven 
   mov rdx, three
   mov ecx, [round_up]
   call apply_mxcsr
;dividing with precition lost, round down
   mov rdi, fmt5
   mov rsi, eleven 
   mov rdx, three
   mov ecx, [round_down]
   call apply_mxcsr
;dividing with precition lost, truncate result bits
   mov rdi, fmt6
   mov rsi, eleven
   mov rdx, three
   mov ecx, [truncate]
   call apply_mxcsr
;exit
   leave
   ret
;------------------------
apply_mxcsr:
   push rbp;16
   mov rbp, rsp
   push rsi;8
   push rdx;16
   push rcx;8
   push rbp; for stack aligning to 16 bytes
   call printf
   pop rbp
   pop rcx
   pop rdx
   pop rsi
   mov     [mxcsr_before], ecx
   ldmxcsr [mxcsr_before];load mxcsr from
   movsd   xmm2, [rsi];divided
   divsd   xmm2, [rdx];divide to [rdx]
   stmxcsr [mxcsr_after];write mxcsr to
   movsd   [xmm], xmm2;for print_xmm fun
   mov     rdi, f_div
   movsd   xmm0, [rsi]
   movsd   xmm1, [rdx]
   call    printf
   call    print_xmm
; print mxcsr
   mov     rdi, f_before
   call    printf
   mov     rdi, [mxcsr_before]
   call    print_mxcsr
   mov     rdi, f_after
   call    printf
   mov     rdi, [mxcsr_after]
   call    print_mxcsr
   leave
   ret
;----------------------------
print_xmm:
   push rbp
   mov rbp, rsp
   mov rdi, hex;print "0x" prefix
   call printf
   mov rcx, 8
   .loop:
      xor rdi, rdi
      mov dil, [xmm+rcx-1];rdi, 8 bit
      push rcx
      push r9
      call print_hex_asm
      pop r9
      pop rcx
      loop .loop
   leave
   ret
;-----------------------------
print_hex_asm:
   section .data
      .zero db "0",0
      .fmt db "%x",0
   section .text
      push rbp
      mov rbp, rsp
      push rax
      push r9
      xor r9, r9
      mov r9b, dil;save arg
      cmp dil, 16
      jmp .tail;if less than 16
      xor rax, rax
      mov rdi, .zero
      call printf
   .tail:
      xor rsi, rsi
      mov sil, r9b
      xor rax, rax
      mov rdi, .fmt
      call printf
      pop r9
      pop rax
      leave
      ret
;----------------------------
print_mxcsr:
   section .data
      .fmt db "%c",0
      .space db " ",0
      .zero db "0",0
      .one db "1",0
   section .text
      push rbp
      mov rbp, rsp
      push rax
      push rcx
      push rdx
      push r9
      mov rcx, 15;16 in counter
      mov r9, rdi; copy argument
      .loop:
         mov rax,rcx
         inc rax
         xor rdi, rdi
         mov dil, 4
         xor rdx, rdx
         div rdi
         cmp rdx, 0
         jne .notspace; if counter%4!=0
         mov rdi, .space
         xor rax, rax
         push rcx
         push r9
         call printf
         pop r9
         pop rcx
      .notspace:
         mov rax, r9; arg in r9
         sar rax, cl; shift right by count
         test rax, 1b
         jz .iszero
         mov rdi, .one
         jmp .continue
      .iszero:
         mov rdi, .zero
      .continue:
         xor rax, rax
         push rcx
         push r9
         call printf
         pop r9
         pop rcx
         dec rcx
         jns .loop
      xor rax, rax
      mov rdi, .fmt
      mov rsi, 10
      call printf
      pop r9
      pop rdx
      pop rcx
      pop rax
      leave
      ret
