;strings.asm
extern printf
section .data
   str1  db  "This is the 1st string.",10,0
   str2  db  "This is the 2nd string.",10,0
   len2  equ $-str2-2
   str21 db  "Comparing strings: The strings are equal.",10,0
   str22 db  "Comparing strings: The strings are not equal.",
         db  " Differ, starting from position: %d.",10,0
   str3  db  "The quick brown fox jumps over the laxy dog.",0
   len3  equ $-str3-2
   str33 db  "Now look at this string: %s",10,0
   str4  db  "y",0
   str44 db  "The character '%s' was found at position %d.",10,0
   str45 db  "The character '%s' was not found in a string.",10,0
   str46 db  "Scanning for the character '%s'.",10,0
section .bss
section .text
   global main
main:
   push rbp
   mov rbp, rsp
;print 2 str
   xor rax, rax
   mov rdi, str1
   call printf
   mov rdi, str2
   call printf
;cmp 2 str
   lea rdi, [str1]
   mov rsi, str2
   mov rdx, len2
   call compare1
   cmp rax, 0
   jnz not_equal1
;equal->print
   mov rdi, str21
   call printf
   call otherversion
not_equal1:
   mov rdi, str22
   mov rsi, rax
   xor rax, rax
   call printf
;cmp, ver2
otherversion:
   lea rdi,[str1]
   mov rsi, str2
   mov rdx, len2
   call compare2
   cmp rax, 0
   jnz not_equal2
;equal
   mov rdi, str21
   call printf
   jmp scanning
not_equal2:
   mov rdi, str22
   mov rsi, rax
   xor rax, rax
   call printf
;find
   mov rdi, str33
   mov rsi, str3
   xor rax, rax
   call printf
   mov rdi, str46
   mov rsi, str4
   xor rax, rax
   call printf
scanning:
   mov rdi, str3; searching element
   lea rsi, [str4];str for search
   mov rdx, len3
   call cscan
   cmp rax, 0
   jz not_found
;founded
   mov rdi, str44
   mov rsi, str4
   mov rdx, rax
   xor rax, rax
   call printf
   jmp exit
not_found:
   mov rdi, str45
   mov rsi, str4
   xor rax, rax
   call printf
exit:
   leave
   ret
;----------------------
compare1:
   mov rcx, rdx
   cld; clear DF
cmpr:
   cmpsb; cmp bytes in rsi, rdi ZF==1, if equal, 0 if not; 
        ;rsi++, rdi++, if D=F=0, else --
   jne notequal; jump if result of cmp is "not equal"
   loop cmpr
   xor rax, rax; is equal
   ret
notequal:
   mov  rax, len2
   dec rcx; rcx in cycle decrementing before check(rcx=strlen-pos)
   sub rax, rcx
   ret
;----------------------
compare2:
   mov rcx, rdx
   cld; DF=0
   repe cmpsb; repeat while equal like a while(ZF)
   je equal2; if equal jump 
   ; if not equal, calculate position from start
   mov rax, len2
   sub rax, rcx
   ret
equal2:
   xor rax, rax
   ret
;---------------------
cscan:
   mov rcx, rdx
   lodsb; mov al, byte[rsi]; ++(-- if(DF)) rsi
   cld; clear DF
   repne scasb; repeat while not equal
   ;scasb compare al and byte[rdi] set ZF, if equal, ++(-- if DF)rdi
   jne nfound
   mov rax, len3
   sub rax, rcx
   ret
nfound:
   xor rax, rax
   ret
