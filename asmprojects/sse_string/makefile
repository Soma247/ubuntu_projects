#makefile for sse_string_length.asm sse_string_search.asm
default: sse_string_length sse_string_search sse_string_cmp_implicit_length\
	sse_string_cmp_explicit_length
sse_string_length: sse_string_length.o 
	gcc -o sse_string_length sse_string_length.o -no-pie 
sse_string_length.o: sse_string_length.asm
	nasm -f elf64 -g -F dwarf sse_string_length.asm -l sse_string_length.lst
sse_string_search: sse_string_search.o 
	gcc -o sse_string_search sse_string_search.o -no-pie 
sse_string_search.o: sse_string_search.asm
	nasm -f elf64 -g -F dwarf sse_string_search.asm -l sse_string_search.lst
sse_string_cmp_implicit_length: sse_string_cmp_implicit_length.o 
	gcc -o sse_string_cmp_implicit_length sse_string_cmp_implicit_length.o\
		-no-pie 
sse_string_cmp_implicit_length.o: sse_string_cmp_implicit_length.asm
	nasm -f elf64 -g -F dwarf sse_string_cmp_implicit_length.asm -l\
		sse_string_cmp_implicit_length.lst
sse_string_cmp_explicit_length: sse_string_cmp_explicit_length.o 
	gcc -o sse_string_cmp_explicit_length sse_string_cmp_explicit_length.o\
		-no-pie 
sse_string_cmp_explicit_length.o: sse_string_cmp_explicit_length.asm
	nasm -f elf64 -g -F dwarf sse_string_cmp_explicit_length.asm -l\
		sse_string_cmp_explicit_length.lst
Phony: default
