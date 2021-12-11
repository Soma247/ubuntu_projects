#makefile for console.asm
console: console.o
	gcc -o console console.o -no-pie
console.o: console.asm
	nasm -f elf64 -g -F dwarf console.asm -l console.lst
.Phony: console
