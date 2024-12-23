all: clean build measure

FILE = tests/ir_test.al

clean:
	rm -rf out/
	mkdir out

build:
	clang -g alan.c -o out/alc

run:
	@echo out/alc $(FILE)
	@echo
	@out/alc $(FILE)
	@echo

measure:
	@echo time out/alc $(FILE)
	@echo
	@fish -c "time out/alc $(FILE)"
	@echo

debug:
	clang -rdynamic -fno-omit-frame-pointer -g alan.c -o out/alc
	lldb out/alc $(FILE) -o run