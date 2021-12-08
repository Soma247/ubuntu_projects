#makefile for printbits.asm
printbits_test: main.o printbits.o
	gcc -o printbits main.o printbits.o -no-pie
main.o: main.asm
	nasm -f elf64 -g -F dwarf main.asm -l main.lst
printbits.o: printbits.asm
	nasm -f elf64 -g -F dwarf printbits.asm -l printbits.lst
.Phony: printbits_test
