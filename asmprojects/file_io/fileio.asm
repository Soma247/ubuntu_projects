;file_io.asm
section .data
;conditional assembling expressions
   CREATE    equ 1
   OVERWRITE equ 1
   APPEND    equ 1
   O_WRITE   equ 1
   READ      equ 1
   O_READ    equ 1
   DELETE    equ 1
;symnames of syscalls
   NR_READ   equ 0
   NR_WRITE  equ 1
   NR_OPEN   equ 2
   NR_CLOSE  equ 3
   NR_LSEEK  equ 8
   NR_CREATE equ 85
   NR_UNLINK equ 87
; create and conditions flags
   O_CREAT  equ 00000100q
   O_APPEND equ 00002000q
; access mode flags
   O_RDONLY equ 000000q
   O_WRONLY equ 000001q
   O_RDWR   equ 000002q
; creation mode flags
   S_IRUSR   equ 00400q; read right for owner
   S_IWUSR   equ 00200q; write right for owner
   NL        equ 0xa
   bufferlen equ 64
   fileName  db "testfile.txt",0
   FD        dq 0; file descriptor
   text1 db "1. Hello...to everyone!",NL,0
   len1 dq $-text1-1
   text2 db "2. Here I am!",NL,0
   len2 dq $-text2-1
   text3 db "3. Alife and kicking!",NL,0
   len3 dq $-text3-1
   text4 db "Adios !!!",NL,0
   len4 dq $-text4-1
   error_cr db "error creation file",NL,0
   error_cl  db "error close file",NL,0
   error_wt db "error writing to file",NL,0
   error_op db "error opening file",NL,0
   error_ap db "error appending to file",NL,0
   error_dl db "error deleting file",NL,0
   error_rd db "error reading file",NL,0
   error_ps db "error positioning in file",NL,0
   success_cr db "File created and opened",NL,0
   success_cl db "File closed",NL,NL,0
   success_wt db "Written to file",NL,0
   success_op db "File opened for R/W",NL,0
   success_ap db "File opened for appending",NL,0
   success_dl db "File deleted",NL,0
   success_rd db "Reading file",NL,0
   success_ps db "Positioned in file",NL,0

section .bss
   buffer resb bufferlen
section .text
   global main
main:
   push rbp
   mov rbp, rsp
%IF CREATE
;creating, opening and closing the file
   mov rdi, fileName
   call createFile
   mov qword[FD], rax; save the descriptor of file
   mov rdi, qword[FD]
   mov rsi, text1
   mov rdx, qword[len1]
   call writeFile
   mov rdi, qword[FD]
   call closeFile
%ENDIF
 
%IF OVERWRITE
; opening,  overwriting and closing file
   mov rdi, fileName
   call openFile
   mov qword[FD], rax
   mov rdi, qword[FD]
   mov rsi, text2
   mov rdx, qword[len2]
   call writeFile
   mov rdi, qword[FD]
   call closeFile
%ENDIF

%IF APPEND
; opening, appending and closing file
   mov rdi, fileName
   call appendFile
   mov qword[FD], rax
   mov rdi, qword[FD]
   mov rsi, text3
   mov rdx, qword[len3]
   call writeFile
   mov rdi, qword[FD]
   call closeFile
%ENDIF

%IF O_WRITE
; opening, rewriting started from a shifted position, closing file
   mov rdi, fileName
   call openFile
   mov qword[FD], rax
   mov rdi, qword[FD]
   mov rsi, qword[len2]
   xor rdx, rdx
   call positionFile
   mov rdi, qword[FD]
   mov rsi, text4
   mov rdx, qword[len4]
   call writeFile
   mov rdi, qword[FD]
   call closeFile
%ENDIF

%IF READ
;opening, reading and closing file
   mov rdi, fileName
   call openFile
   mov qword[FD], rax
   mov rdi, qword[FD]
   mov rsi, buffer
   mov rdx, bufferlen
   call readFile
   mov rdi, rax
   call printString
   mov rdi, qword[FD]
   call closeFile
%ENDIF

%IF O_READ
; opening, reading started from position and closing file
   mov rdi, fileName
   call openFile
   mov qword[FD], rax
   mov rdi, qword[FD]
   mov rsi, qword[len2]
   mov rdx, 0
   call positionFile
   mov rdi, qword[FD]
   mov rsi, buffer
   mov rdx, 10
   call readFile
   mov rdi, rax
   call printString
   mov rdi, qword[FD]
   call closeFile
%ENDIF

%IF DELETE
;deleting file
   mov rdi, fileName
   call deleteFile
%ENDIF
   leave
   ret
;-------------------------
global readFile
readFile:
   mov rax, NR_READ
   syscall;count of readed chars in rax
   cmp rax, 0
   jl readerror;nothing readed
   mov byte[rsi+rax], 0; end of char sequence
   mov rax, rsi; ptr to buffer in rax
   mov rdi, success_rd; success code in rdi
   push rax; caller-saved register
   call printString
   pop rax
   ret
readerror:
   mov rdi, error_rd; error code in rdi
   call printString
   ret
;---------------------------
global deleteFile
deleteFile:
   mov rax, NR_UNLINK
   syscall
   cmp rax, 0
   jl deleteerror
   mov rdi, success_dl
   call printString
   ret
deleteerror:
   mov rdi, error_dl
   call printString
   ret
;---------------------------
global appendFile
appendFile:
   mov rax, NR_OPEN
   mov rsi, O_RDWR|O_APPEND
   syscall
   cmp rax, 0
   jl appenderror
   mov rdi, success_ap
   push rax
   call printString
   pop rax
   ret
appenderror:
   mov rdi, error_ap
   call printString
   ret
;---------------------------
global openFile
openFile:
   mov rax, NR_OPEN
   mov rsi, O_RDWR
   syscall
   cmp rax, 0
   jl openerror
   mov rdi, success_op
   push rax
   call printString
   pop rax
   ret
openerror:
   mov rdi, error_op
   call printString
   ret
;---------------------------
global writeFile
writeFile:
   mov rax, NR_WRITE
   syscall
   cmp rax, 0
   jl writeerror
   mov rdi, success_wt
   call printString
   ret
writeerror:
   mov rdi, error_wt
   call printString
   ret
;---------------------------
global positionFile
positionFile:
   mov rax, NR_LSEEK
   syscall
   cmp rax, 0
   jl poserror
   mov rdi, success_ps
   call printString
   ret
poserror:
   mov rdi, error_ps
   call printString
   ret
;---------------------------
global closeFile
closeFile:
   mov rax, NR_CLOSE
   syscall
   cmp rax, 0
   jl closeerror
   mov rdi, success_cl
   call printString
   ret
closeerror:
   mov rdi, error_cl
   call printString
   ret
;---------------------------
global createFile
createFile:
   mov rax, NR_CREATE
   mov rsi, S_IRUSR|S_IWUSR; access mode flags: rw for owner
   syscall
   cmp rax, 0 ; FD in rax if creating is success
   jl createerror
   mov rdi, success_cr
   push rax
   call printString
   pop rax
   ret
createerror:
   mov rdi, error_cr
   call printString
   ret
;---------------------------
global printString
printString:
   mov r12, rdi
   xor rdx, rdx
strLoop:
   cmp byte[r12], 0
   je strDone
   inc rdx
   inc r12
   jmp strLoop
strDone:
   cmp rdx, 0; len==0
   je prtDone
   mov rsi, rdi
   mov rax, 1
   mov rdi, 1
   syscall
prtDone:
   ret
   
