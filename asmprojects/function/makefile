#makefile for area.asm area2.asm
default: area area2
area: area.o
	gcc -o area area.o -no-pie
area2: area2.o
	gcc -o area2 area2.o -no-pie
area.o: area.asm
	nasm -f elf64 -g -F dwarf area.asm -l area.lst
area2.o: area2.asm
	nasm -f elf64 -g -F dwarf area2.asm -l area2.lst
.Phony: area area2
