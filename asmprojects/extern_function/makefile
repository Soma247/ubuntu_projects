#makefile for extern_functiondefault:
main: circle.o main.o rect.o
	gcc -o extern_function main.o circle.o rect.o -no-pie
main.o: main.asm
	nasm -f elf64 -g -F dwarf main.asm -l main.lst
circle.o: circle.asm
	nasm -f elf64 -g -F dwarf circle.asm -l circle.lst
rect.o: rect.asm
	nasm -f elf64 -g -F dwarf rect.asm -l rect.lst
.Phony: main 
