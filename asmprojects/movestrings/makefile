#makefile for move_strings.asm
default: move_strings strings
move_strings: move_strings.o
	gcc -o move_strings move_strings.o -no-pie
move_strings.o: move_strings.asm
	nasm -f elf64 -g -F dwarf move_strings.asm -l move_strings.lst
strings: strings.o
	gcc -o strings strings.o -no-pie
strings.o: strings.asm
	nasm -f elf64 -g -F dwarf strings.asm -l strings.lst
.Phony: move_strings
