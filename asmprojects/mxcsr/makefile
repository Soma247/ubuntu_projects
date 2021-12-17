#makefile for mxcsr.asm
mxcsr: mxcsr.o 
	gcc -o mxcsr mxcsr.o print_hex.o -no-pie 
mxcsr.o: mxcsr.asm
	nasm -f elf64 -g -F dwarf mxcsr.asm -l mxcsr.lst
.Phony: mxcsr
