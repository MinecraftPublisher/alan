all: nasm_build objdump

fasm_build:
	fasm test.s

nasm_build:
	nasm -felf64 test.s
	ld test.o -o test
	rm -rf test.o
	@echo ./test
	@echo
	@./test

objdump:
	objdump -d test
	@objdump -d test | ./disbin.js