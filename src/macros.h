#pragma once

//

#define p_cat(a, b) a##b
#define cat(a, b)   p_cat(a, b)
#define A(type)                                                                                    \
    struct {                                                                                       \
        i32   size;                                                                                \
        i32   unit;                                                                                \
        type *array;                                                                               \
    }
#define P(i) i *
#define var  __auto_type

#define size(arr)           ((arr)->size - 1)
#define mem(size)           alloc(mem, size)
#define scratch(size)       alloc(scratch, size)
#define fn(type, name, ...) type name(__VA_ARGS__ __VA_OPT__(, ) Arena *mem)

#define ret(...)           newx(__VA_ARGS__, ret2, ret1)(__VA_ARGS__)
#define retx(a, b, c, ...) c
#define ret2(type, count)  (A(type) *) __new_array(count, sizeof(type), mem)
#define ret1(type)         (type *) alloc(mem, sizeof(type))

#define new(...)           newx(__VA_ARGS__, new2, new1)(__VA_ARGS__)
#define newx(a, b, c, ...) c
#define new2(type, count)  (A(type) *) __new_array(count, sizeof(type), scratch)
#define new1(type)         (type *) alloc(scratch, sizeof(type))
#define ground()                                                                                   \
    Arena  scratch_space = { 0 };                                                                  \
    Arena *scratch       = &scratch_space
#define release() arena_free(scratch)

#define null NULL

#ifdef arena_debug
    #define adeb(...) __VA_ARGS__
#else
    #define adeb(...)
#endif

#define ARENA_SIZE_UNIT 8192

#define push(arr, data, _mem)                                                                      \
    ({                                                                                             \
        extend((void *) arr, 1, _mem);                                                             \
        arr->array[ arr->size - 1 ] = data;                                                        \
    })

#define cur(...) (eof() ? 0 : (con->str.array[ con->current __VA_OPT__(+) __VA_ARGS__ ]))
#define skip()   (con->str.array[ con->current++ ])
#define eof()    (con->str.size - 1 <= con->current)

#define is_num(a)           (a >= '0' && a <= '9')
#define is_ref_firstchar(a) ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || (a == '_'))
#define is_refchar(a)       (is_ref_firstchar(a) || is_num(a))

#ifdef enable_debug
    #define debug(name)                                                                            \
        ({                                                                                         \
            fprintf(stderr, "Parsing " #name " index %li\n", con->current);                        \
            fprintf(stderr, "```\n%s\n```\n\n", &(con->str.array[ con->current ]));                \
            /*dump_stack();*/                                                                      \
        })
#else
    #define debug(name)
#endif

// ANSI color codes
#define C_RESET   "\033[0m"
#define C_BLACK   "\033[30m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_WHITE   "\033[37m"

// Bold colors
#define C_BOLD_BLACK   "\033[1;30m"
#define C_BOLD_RED     "\033[1;31m"
#define C_BOLD_GREEN   "\033[1;32m"
#define C_BOLD_YELLOW  "\033[1;33m"
#define C_BOLD_BLUE    "\033[1;34m"
#define C_BOLD_MAGENTA "\033[1;35m"
#define C_BOLD_CYAN    "\033[1;36m"
#define C_BOLD_WHITE   "\033[1;37m"

// Bright colors
#define C_BRIGHT_BLACK   "\033[90m"
#define C_BRIGHT_RED     "\033[91m"
#define C_BRIGHT_GREEN   "\033[92m"
#define C_BRIGHT_YELLOW  "\033[93m"
#define C_BRIGHT_BLUE    "\033[94m"
#define C_BRIGHT_MAGENTA "\033[95m"
#define C_BRIGHT_CYAN    "\033[96m"
#define C_BRIGHT_WHITE   "\033[97m"

// Macro to print colored text
#define COLOR_TEXT(color, text) color text C_RESET

// Macros for convenience
#define black(text)   COLOR_TEXT(C_BLACK, text)
#define red(text)     COLOR_TEXT(C_RED, text)
#define green(text)   COLOR_TEXT(C_GREEN, text)
#define yellow(text)  COLOR_TEXT(C_YELLOW, text)
#define blue(text)    COLOR_TEXT(C_BLUE, text)
#define magenta(text) COLOR_TEXT(C_MAGENTA, text)
#define cyan(text)    COLOR_TEXT(C_CYAN, text)
#define white(text)   COLOR_TEXT(C_WHITE, text)

#define bold_black(text)   COLOR_TEXT(C_BOLD_BLACK, text)
#define bold_red(text)     COLOR_TEXT(C_BOLD_RED, text)
#define bold_green(text)   COLOR_TEXT(C_BOLD_GREEN, text)
#define bold_yellow(text)  COLOR_TEXT(C_BOLD_YELLOW, text)
#define bold_blue(text)    COLOR_TEXT(C_BOLD_BLUE, text)
#define bold_magenta(text) COLOR_TEXT(C_BOLD_MAGENTA, text)
#define bold_cyan(text)    COLOR_TEXT(C_BOLD_CYAN, text)
#define bold_white(text)   COLOR_TEXT(C_BOLD_WHITE, text)

#define bright_black(text)   COLOR_TEXT(C_BRIGHT_BLACK, text)
#define bright_red(text)     COLOR_TEXT(C_BRIGHT_RED, text)
#define bright_green(text)   COLOR_TEXT(C_BRIGHT_GREEN, text)
#define bright_yellow(text)  COLOR_TEXT(C_BRIGHT_YELLOW, text)
#define bright_blue(text)    COLOR_TEXT(C_BRIGHT_BLUE, text)
#define bright_magenta(text) COLOR_TEXT(C_BRIGHT_MAGENTA, text)
#define bright_cyan(text)    COLOR_TEXT(C_BRIGHT_CYAN, text)
#define bright_white(text)   COLOR_TEXT(C_BRIGHT_WHITE, text)

#define pop_value "32"
#define POS_ALIGN "\033[" pop_value "G" C_BRIGHT_BLACK

#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) < (b)) ? (b) : (a)

#define EI_NIDENT 16
