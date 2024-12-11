all: clean compiler build_test analyze
build: clean compiler parser

clean:
	rm -rf out
	mkdir out
	bun install

parser:
	peggy parser.pegjs --format es

watch_parser:
	nodemon  -w parser.pegjs --exec "clear; peggy parser.pegjs --format es"

compiler:
	bun build main.ts --outfile out/alc --compile

build_test:
	out/alc tests/test.al out/test

test:
	@echo out/test
	@echo
	@out/test
	@echo

analyze:
	realtime "out/test > /dev/null" > analysis.txt
	@echo
	@echo
	@fish -c "time out/test > /dev/null"
	@echo
	@echo
	gprof out/test >> analysis.txt