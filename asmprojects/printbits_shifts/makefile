#makefile for printbits.asm
default: shiftbits1 shiftbits2 bts_btr_bt
shiftbits1: shiftbits1.o printbits.o
	gcc -o shiftbits1 shiftbits1.o printbits.o -no-pie
shiftbits1.o: shiftbits1.asm
	nasm -f elf64 -g -F dwarf shiftbits1.asm -l shiftbits1.lst
shiftbits2: shiftbits2.o
	gcc -o shiftbits2 shiftbits2.o -no-pie
shiftbits2.o: shiftbits2.asm
	nasm -f elf64 -g -F dwarf shiftbits2.asm -l shiftbits2.lst
bts_btr_bt:bts_btr_bt.o printbits.o
	gcc -o bts_btr_bt bts_btr_bt.o printbits.o -no-pie
bts_btr_bt.o:bts_btr_bt.asm
	nasm -f elf64 -g -F dwarf bts_btr_bt.asm -l bts_btr_bt.lst
printbits.o: printbits.asm
	nasm -f elf64 -g -F dwarf printbits.asm -l printbits.lst
.Phony: default
