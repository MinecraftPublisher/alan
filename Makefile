all: clean build count measure

FILE = src/self/allocator.al
MAIN_SRC = src/alan.c
OUTPUT_FOLDER = out/
OUTPUT_FILE = alan

clean:
	rm -rf $(OUTPUT_FOLDER)
	mkdir $(OUTPUT_FOLDER)

build:
	clang $(MAIN_SRC) -o $(OUTPUT_FOLDER)$(OUTPUT_FILE)

run:
	@echo $(OUTPUT_FOLDER)$(OUTPUT_FILE) $(FILE)
	@echo
	@$(OUTPUT_FOLDER)$(OUTPUT_FILE) $(FILE)

measure:
	@echo time $(OUTPUT_FOLDER)$(OUTPUT_FILE) $(FILE)
	@echo
	@fish -c "time $(OUTPUT_FOLDER)$(OUTPUT_FILE) $(FILE)"

debug:
	clang -rdynamic -fno-omit-frame-pointer -g $(MAIN_SRC) -o $(OUTPUT_FOLDER)$(OUTPUT_FILE)
	lldb $(OUTPUT_FOLDER)$(OUTPUT_FILE) $(FILE) -o run

count:
	@echo
	@echo "echo lines chars name"
	@echo "find src -name '*.h' -o -name '*.c' | xargs wc -lc"
	@echo
	@echo lines chars name
	@find src -name '*.h' -o -name '*.c' | xargs wc -lc
	@echo