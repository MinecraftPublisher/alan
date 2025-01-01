all: nasm_build objdump

fasm_build:
	fasm test.s

nasm_build:
	nasm -f elf64 test.s
	ld test.o -o test -melf_x86_64
	rm -rf test.o

run:
	@echo ./test
	@echo
	@./test

objdump:
	objdump -d test
	@# objdump -d test | ./disbin.js
	@echo
	@echo