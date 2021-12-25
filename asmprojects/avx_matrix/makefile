#makefile for alive.asm
default: matrix4x4 transposition4x4
matrix4x4: matrix4x4.o printm4x4.o
	gcc -o matrix4x4 matrix4x4.o printm4x4.o -no-pie
matrix4x4.o: matrix4x4.asm
	nasm -f elf64 -g -F dwarf matrix4x4.asm -l matrix4x4.lst
transposition4x4: transposition4x4.o printm4x4.o
	gcc -o transposition4x4 transposition4x4.o printm4x4.o -no-pie
transposition4x4.o: transposition4x4.asm
	nasm -f elf64 -g -F dwarf transposition4x4.asm -l transposition4x4.lst
printm4x4.o: matrix4x4.asm
	nasm -f elf64 -g -F dwarf printm4x4.asm -l printm4x4.lst
.Phony: default
