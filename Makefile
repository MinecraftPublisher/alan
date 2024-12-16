all: clean build measure

FILE = tests/example.al

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
	lldb out/parser tests/example.al -o run