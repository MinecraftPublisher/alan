all: clean hell build count measure

FILE = src/self/allocator.al
MAIN_SRC = src/alan.c
OUTPUT_FOLDER = out/
OUTPUT_FILE = alan

clean:
	rm -rf $(OUTPUT_FOLDER)
	mkdir $(OUTPUT_FOLDER)

# TODO: Figure out why address sanitizer breaks the program at arena_free

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

FIND_CMD = find src -name '*.h' -o -name '*.c' -o -name '*.al' | xargs wc -lc

hell:
	clang -Wall -Werror -g $(MAIN_SRC) -o $(OUTPUT_FOLDER)$(OUTPUT_FILE)
	@echo
	@echo 'You escaped hell!'

count:
	@echo
	@echo "echo lines chars name"
	@echo "$(FIND_CMD)"
	@echo
	@echo lines chars name
	@$(FIND_CMD)
	@echo