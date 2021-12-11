#makefile for fileio.asm
fileio: fileio.o
	gcc -o fileio fileio.o -no-pie
fileio.o: fileio.asm
	nasm -f elf64 -g -F dwarf fileio.asm -l fileio.lst
.Phony: fileio
