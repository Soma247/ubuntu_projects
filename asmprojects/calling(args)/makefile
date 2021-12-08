#makefile for alive.asm
args: args.o
	gcc -o args args.o -no-pie
args.o: args.asm
	nasm -f elf64 -g -F dwarf args.asm -l args.lst
.Phony: args
