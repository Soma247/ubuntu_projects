#makefile for alive.asm
floating_point: floating_point.o
	gcc -o floating_point floating_point.o -no-pie
floating_point.o: floating_point.asm
	nasm -f elf64 -g -F dwarf floating_point.asm -l floating_point.lst
.Phony: floating_point
