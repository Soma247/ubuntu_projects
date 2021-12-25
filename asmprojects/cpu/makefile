#makefile for cpu.asm
default: cpu avx timer
cpu: cpu.o
	gcc -o cpu cpu.o -no-pie
avx: avx.o
	gcc -o avx avx.o -no-pie
timer: timer.o
	gcc -o timer timer.o -no-pie
cpu.o: cpu.asm
	nasm -f elf64 -g -F dwarf cpu.asm -l cpu.lst
avx.o: avx.asm
	nasm -f elf64 -g -F dwarf avx.asm -l avx.lst
timer.o: timer.asm
	nasm -f elf64 -g -F dwarf timer.asm -l timer.lst
.Phony: default
