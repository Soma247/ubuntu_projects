#makefile for shuffle.asm
default: shuffle string_masks search_from_range search_substr
shuffle: shuffle.o 
	gcc -o shuffle shuffle.o -no-pie 
shuffle.o: shuffle.asm
	nasm -f elf64 -g -F dwarf shuffle.asm -l shuffle.lst
string_masks: string_masks.o print_mask.o print16b.o 
	gcc -o string_masks string_masks.o print_mask.o print16b.o -no-pie 
string_masks.o: string_masks.asm
	nasm -f elf64 -g -F dwarf string_masks.asm -l string_masks.lst
search_from_range: search_from_range.o
	gcc -o search_from_range search_from_range.o print_mask.o print16b.o -no-pie
search_from_range.o: search_from_range.asm
	nasm -f elf64 -g -F dwarf search_from_range.asm -l search_from_range.lst
search_substr: search_substr.o
	gcc -o search_substr search_substr.o print_mask.o print16b.o -no-pie
search_substr.o:search_substr.asm
	nasm -f elf64 -g -F dwarf search_substr.asm -l search_substr.lst
print_mask.o:print_mask.asm
	nasm -f elf64 -g -F dwarf print_mask.asm -l print_mask.lst
print16b.o: print16b.asm
	nasm -f elf64 -g -F dwarf print16b.asm -l print16b.lst
.Phony: default
