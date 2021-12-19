#makefile for sse_unaligned.asm sse_integer.asm
sse_integer: sse_integer.o 
	gcc -o sse_integer sse_integer.o -no-pie 
sse_integer.o: sse_integer.asm
	nasm -f elf64 -g -F dwarf sse_integer.asm -l sse_integer.lst

.Phony: sse_integer 
