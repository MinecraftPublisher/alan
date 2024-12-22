all: clean build measure

FILE = tests/ir_test.al

clean:
	rm -rf out/
	mkdir out

build:
	clang -g parser.c -o out/parser

run:
	@echo out/parser $(FILE)
	@echo
	@out/parser $(FILE)
	@echo

measure:
	@echo time out/parser $(FILE)
	@echo
	@fish -c "time out/parser $(FILE)"
	@echo

debug:
	clang -rdynamic -fno-omit-frame-pointer -g parser.c -o out/parser
	lldb out/parser $(FILE) -o run