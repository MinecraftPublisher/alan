// #define arena_debug
// #define enable_debug

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef unsigned char byte;
typedef long          i32;
typedef long long     i64;
#define null NULL
char nullchar = '\0';

typedef struct Array {
    i32   size;
    i32   unit;
    byte *array;
} *A;

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

typedef struct Block {
    i32   size;
    i32   used;
    byte *data;
} Block;

typedef struct {
    byte initialized;
    A(Block) * blocks;
} Arena;

typedef A(char) string;

void dump_stack() {
    void  *array[ 100 ];
    size_t size;
    char **strings;

    size    = backtrace(array, 100);
    strings = backtrace_symbols(array, size);

    if (strings != NULL) {
        printf("    Stack trace (%zd frames):\n", size);
        for (size_t i = 0; i < size; i++) { printf("        #%zu: %s\n", i, strings[ i ]); }
        free(strings);
    } else {
        printf("Failed to get stack trace\n");
    }
}

#ifdef arena_debug
    #define adeb(...) __VA_ARGS__
#else
    #define adeb(...)
#endif

void arena_free(Arena *arena) {
    adeb(printf("Free(%p)\n", arena));
    adeb(dump_stack());
    if (arena->initialized) {
        for (i32 i = 0; i < arena->blocks->size; i++) {
            adeb(printf("Block.Free(%p)\n", arena->blocks->array[ i ].data));
            munmap(arena->blocks->array[ i ].data, arena->blocks->array[ i ].size);
        }
        munmap(arena->blocks->array, arena->blocks->size * arena->blocks->unit);
        munmap(arena->blocks, sizeof(A(Block)));
        arena->initialized = 0;
        arena->blocks      = NULL;
    }
}

#define SIZE_UNIT 8192

void *alloc(Arena *arena, i32 size) {
    if (!arena->initialized) {
        arena->initialized = 1;
        arena->blocks      = mmap(
            NULL, sizeof(A(Block)), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (arena->blocks == NULL) {
            fprintf(stderr, "Memory allocation failed in alloc!\n");
            exit(EXIT_FAILURE);
        }
        arena->blocks->size  = 0;
        arena->blocks->unit  = sizeof(Block);
        arena->blocks->array = NULL;
    }

    for (i32 i = 0; i < arena->blocks->size; i++) {
        Block *block = &arena->blocks->array[ i ];
        if (block->size - block->used >= size) {
            void *ptr = &block->data[ block->used ];
            block->used += size;
            adeb(printf("Alloc(%p, %li, %p)\n", arena, size, ptr));
            return ptr;
        }
    }

    Block new_block;
    new_block.size = ((size * 2) > SIZE_UNIT) ? (size * 2) : SIZE_UNIT;

    new_block.used = size;
    new_block.data
        = mmap(null, new_block.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_block.data == NULL) {
        fprintf(stderr, "Memory allocation failed in alloc!\n");
        exit(EXIT_FAILURE);
    }
    memset(new_block.data, 0, new_block.size); // bad for performance?

    (arena->blocks->size)++;
    var previous         = arena->blocks->array;
    arena->blocks->array = mmap(
        NULL,
        sizeof(Block) * arena->blocks->size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    // arena->blocks->array = realloc(arena->blocks->array, sizeof(Block) * arena->blocks->size);
    if (arena->blocks->array == NULL) {
        fprintf(stderr, "Memory allocation failed in alloc (realloc)!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(arena->blocks->array, previous, sizeof(Block) * (arena->blocks->size - 1));
    munmap(previous, sizeof(Block) * (arena->blocks->size - 1));

    arena->blocks->array[ arena->blocks->size - 1 ] = new_block;

    adeb(printf("Block.New(%p, %li, %p)\n", arena, size, new_block.data));
    return new_block.data;
}

string str(char *in, i32 len, Arena *arena) {
    var space = (char *) alloc(arena, len);

    for (i32 i = 0; i < len; i++) space[ i ] = in[ i ];
    space[ len ] = 0;

    return (string) { .unit = 1, .size = len, .array = space };
}

fn(A, __new_array, i32 count, i32 unit) {
    A arr      = alloc(mem, sizeof(struct Array));
    arr->size  = count;
    arr->unit  = unit;
    arr->array = alloc(mem, sizeof(byte) * (count + 1) * unit);

    return arr;
}

void extend(A array, i32 count, Arena *mem) {
    var old      = array->array;
    array->array = alloc(mem, sizeof(byte) * (array->size + count + 1) * array->unit);
    memcpy(array->array, old, array->size * array->unit);
    array->size += count;
}

void *copy(void *_array, Arena *mem) {
    A array          = _array;
    A new_array      = alloc(mem, sizeof(struct Array));
    new_array->size  = array->size;
    new_array->unit  = array->unit;
    new_array->array = alloc(mem, (new_array->size + 1) * new_array->unit);
    memcpy(new_array->array, array->array, new_array->size * new_array->unit);

    return new_array;
}

#define push(arr, data, _mem)                                                                      \
    ({                                                                                             \
        extend((void *) arr, 1, _mem);                                                             \
        arr->array[ arr->size - 1 ] = data;                                                        \
    })

typedef i32 symbol;

struct Entry {
    enum { etnum, etstr, etfn } type;
    union {
        i32    num;
        string str;
        struct {
            i32   name;
            short arg_count;
            byte  implemented;
            char *types;
            char  ret;
        } fn;
    } value;
    i32 references;
};

struct _ctx {
    string str;
    i32    current;
    A(struct Entry) * literals;
    A(char *) * symbols;
};

typedef struct _ctx *ctx;

typedef enum { exp_void = 3, exp_block = 2, exp_num = -1, exp_list = 1, exp_any = 0 } exp_type;

typedef struct i *i;
struct i {
    enum { tstr, tnum, tcode, tcall, tref, tfn, tchr, tembd } type;
    union {
        i32 str_id;
        i32 num_id;
        struct {
            i32 size;
            i32 unit;
            i  *array;
            A(char *) * symbols;
        } *block;
        struct {
            symbol name;
            A(i) * args;
        } call;
        symbol ref;
        struct {
            symbol   name;
            symbol   entry;
            exp_type type;
            i        block;
            A(symbol) * args;
        } fn;
        char    chr;
        string *direct;
    } value;
};

byte starts_with(ctx con, char *predicate) {
    return strcmp(&con->str.array[ con->current ], predicate) == 0;
}

#define cur(...) (eof() ? 0 : (con->str.array[ con->current __VA_OPT__(+) __VA_ARGS__ ]))
#define skip()   (con->str.array[ con->current++ ])
#define eof()    (con->str.size - 1 <= con->current)

void parse_comment(ctx con) {
    if (cur() == '#') { while (skip() != '\n' && !eof()); }
}

void parse_whitespace(ctx con) {
    var c = cur();
    while ((c == ' ' || c == '\t' || c == '\n')) {
        skip();
        c = cur();
    }
}

void parse_null(ctx con) {
    var prev = con->current - 1;

    while (con->current != prev) {
        prev = con->current;
        parse_comment(con);
        parse_whitespace(con);
    }
}

#define is_num(a)           (a >= '0' && a <= '9')
#define is_ref_firstchar(a) ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || (a == '_'))
#define is_refchar(a)       (is_ref_firstchar(a) || is_num(a))

const var ir_error       = (void *) -2;
const var analyzer_error = (void *) -1;

void error(ctx con, char *text) {
    // TODO: Improve error reporting, maybe add stack trace?
    if (con == analyzer_error) printf("Inspector Error: %s\n", text);
    else if (con == ir_error)
        printf("Tourist Error: %s\n", text);
    else {
        for (i32 i = 0; i < con->str.size; i++) { putchar(con->str.array[ i ]); }
        printf("\nLibrarian Error: %s\nIndex: %li\n\n", text, con->current);
    }
    exit(1);
}

string *read_file(char *name, Arena *mem) {
    FILE *file = fopen(name, "r");
    if (file == null) {
        printf("Could not open file %s\n", name);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    var size = ftell(file);
    fseek(file, 0, SEEK_SET);

    var text = (string *) ret(char, size + 1);

    // Read the entire file
    fread(text->array, 1, size, file);
    text->array[ size ] = '\0'; // Null terminate the string

    fclose(file);

    return text;
}

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

// ------------------------
// ------- PRINTER --------
// ------------------------

void print_i(i val, i32 indent, ctx con, byte print_sym) {
    if (val == null) printf("\r");

    else if (val->type == tref) {
        if (print_sym) printf("<$%li>", val->value.ref);
        else { printf("%s", con->symbols->array[ labs(val->value.ref) ]); }
    }

    else if (val->type == tchr) {
        printf("'%c'", val->value.chr);
    }

    else if (val->type == tstr) {
        if (print_sym) printf("<\"%li>", val->value.str_id);
        else { printf("\"%s\"", con->literals->array[ val->value.str_id ].value.str.array); }
    }

    else if (val->type == tnum) {
        if (print_sym) printf("<#%li>", val->value.str_id);
        else { printf("%li", con->literals->array[ val->value.str_id ].value.num); }
    }

    else if (val->type == tfn) {
        if (print_sym) printf("\nfn[%i] <$%li>(", val->value.fn.type, val->value.fn.name);
        else {
            printf(
                "fn[%s] %s (",
                val->value.fn.type == exp_num    ? "num"
                : val->value.fn.type == exp_list ? "list"
                                                 : "void",
                con->symbols->array[ val->value.fn.name ]);
        }
        for (i32 i = 0; i < val->value.fn.args->size; i++) {
            if (print_sym) printf("<$%li>", val->value.fn.args->array[ i ]);
            else { printf("%s", con->symbols->array[ labs(val->value.fn.args->array[ i ]) ]); }
            if (i + 1 < val->value.fn.args->size) printf(", ");
        }
        printf(") ");

        print_i(val->value.fn.block, indent, con, print_sym);
        printf(";\n");
    }

    else if (val->type == tcall) {
        if (print_sym) printf("<$%li>", val->value.call.name);
        else
            printf("%s", con->symbols->array[ val->value.call.name ]);
        for (i32 i = 0; i < val->value.call.args->size; i++) {
            printf(" ");
            print_i(val->value.call.args->array[ i ], indent, con, print_sym);
        }
        printf(";");
    }

    else if (val->type == tcode) {
        printf("[ ");
        if (val->value.block->size > 1) printf("\n");
        for (i32 i = 0; i < val->value.block->size; i++) {
            if (val->value.block->array[ i ] == null) continue;

            if (val->value.block->size > 1) {
                for (i32 sp = 0; sp <= indent; sp++) printf("    ");
            }

            print_i(val->value.block->array[ i ], indent + 1, con, print_sym);
            if (i + 1 < val->value.block->size && val->value.block->size > 1) printf("\n");
        }

        if (val->value.block->size > 1) {
            printf("\n");
            for (i32 sp = 0; sp < indent; sp++) printf("    ");
        } else {
            printf(" ");
        }

        printf("]");
    }

    else if (val->type == tembd) {
        printf("{{ %s }}", val->value.direct->array);
    }

    else {
        printf("{unknown - %i}", val->type);
    }
}

// ------------------------
// -------- PARSER --------
// ------------------------

const unsigned long stdlib_size;

// zero shows maximum optimization level. sacrifice compile-time for run-time.
enum {
    OPT_AGGRESSIVE = 0,
    OPT_BASIC      = 1,
    OPT_MEDIUM     = 2,
    OPT_NONE       = 10
} parser_optimizations
    = OPT_AGGRESSIVE;

fn(symbol, parse_ref_str, ctx con) {
    if (!is_ref_firstchar(cur())) return -1;
    ground();

    var result         = (string *) ret(char, 1);
    result->array[ 0 ] = skip();

    while (is_refchar(cur())) { push(result, skip(), mem); }

    push(result, nullchar, mem);

    symbol index = -1;

    for (i32 i = 0; i < con->symbols->size; i++) {
        if (con->symbols->array[ i ] == null) continue;
        // printf("%s ?= %s\n", con->symbols->array[i], result->array);
        if (!strcmp(result->array, con->symbols->array[ i ])) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        index = con->symbols->size;
        push(con->symbols, result->array, mem);
    }

    release();
    return index;
}

fn(i, parse_ref, ctx con) {
    var out = parse_ref_str(con, mem);
    if (out == -1) return null;
    debug(ref);

    i val = ret(struct i);

    val->type      = tref;
    val->value.ref = out;

    return val;
}

fn(i, parse_char, ctx con) {
    if (cur() != '\'') return null;
    debug(char);

    skip();
    var value = skip();
    if (skip() != '\'') error(con, "Expected closing ' when parsing character!");

    i val          = ret(struct i);
    val->type      = tchr;
    val->value.chr = value;

    return val;
}

fn(i, parse_num, ctx con) {
    if (!is_num(cur())) return null;
    ground();
    debug(num);

    var txt = new (char, 0);
    push(txt, skip(), scratch);

    while (is_num(cur())) push(txt, skip(), scratch);

    i val = ret(struct i);

    val->type = tnum;

    var out_val = atoi(txt->array);

    if (parser_optimizations <= OPT_MEDIUM) {
        for (i32 i = 0; i < con->literals->size; i++) {
            var cur = con->literals->array[ i ];

            if (cur.type == etnum && cur.value.num == out_val) {
                val->value.num_id = i;
                con->literals->array[ i ].references++;

                release();
                return val;
            }
        }
    }

    var entry = (struct Entry) { .type = etnum, .value.num = out_val, .references = 1 };
    push(con->literals, entry, mem);
    val->value.num_id = con->literals->size - 1;

    release();
    return val;
}

fn(i, parse_str, ctx con) {
    if (cur() != '"') return null;
    ground();
    debug(str);

    var txt = new (char, 0);
    skip();
    // push(txt, &skip(), scratch);

    while ((cur() != '"' || ((con->str.array[ con->current - 1 ]) == '\\')) && !eof()) {
        if (cur() == '"' && ((con->str.array[ con->current - 1 ]) == '\\')) {
            txt->array[ txt->size - 1 ] = '"';
            skip();
        } else {
            push(txt, skip(), scratch);
        }
    }

    if (eof() && cur() != '"') {
        error(con, "Expected closing \" when parsing string, but got end-of-file instead...");
    }

    skip();

    i val = ret(struct i);

    val->type = tstr;

    push(txt, nullchar, scratch);

    var entry                                     = (struct Entry) { .type       = etstr,
                                                                     .value.str  = str(txt->array, txt->size, mem),
                                                                     .references = 1 };
    entry.value.str.array[ entry.value.str.size ] = nullchar;

    if (parser_optimizations <= OPT_MEDIUM) {
        for (i32 i = 0; i < con->literals->size; i++) {
            var cur = con->literals->array[ i ];

            if (cur.type == etstr && !strcmp(cur.value.str.array, txt->array)) {
                val->value.str_id = i;
                con->literals->array[ i ].references++;

                release();
                return val;
            }
        }
    }

    push(con->literals, entry, mem);

    val->value.str_id = con->literals->size - 1;

    release();
    return val;
}

fn(string *, trim_str, string input) {
#define whitespace(x)                                                                              \
    ((input.array[ x ] == ' ' || input.array[ x ] == '\t' || input.array[ x ] == '\n'))

    i32 start_i = 0;
    while (whitespace(start_i)) { start_i++; }

    i32 end_i = input.size - 1;
    while ((end_i >= start_i) && whitespace(end_i)) { end_i--; }

    i32 size = end_i - start_i + 1;

    string *new = (void *) ret(char, size);

    i32 o_i = 0;
    for (i32 i = start_i; i <= end_i; i++) { new->array[ o_i++ ] = input.array[ i ]; }

    return new;
#undef whitespace
}

fn(i, parse_embd, ctx con) {
    if (cur() != '{') return null;
    ground();
    debug(embed);

    skip();
    if (skip() != '{') error(con, "Expected {{ when parsing direct embedding!");

    i val      = ret(struct i);
    val->type  = tembd;
    var result = (string *) new (char, 0);

    i32 depth = 0;
    while (cur() != '}' || cur(1) != '}' || depth > 0) {
        if (cur() == '{') depth++;
        if (cur() == '}') depth--;
        push(result, skip(), mem);
    }

    skip();
    skip();
    result->array[ result->size ] = nullchar;

    val->value.direct = trim_str(*result, mem);

    release();
    return val;
}

i parse_block(ctx con, Arena *mem);

fn(i, parse_expr, ctx con) {
    // debug(expr);
    var value = parse_str(con, mem);
    if (value == null) value = parse_embd(con, mem);
    if (value == null) value = parse_num(con, mem);
    if (value == null) value = parse_char(con, mem);
    if (value == null) value = parse_ref(con, mem);
    if (value == null) value = parse_block(con, mem);

    return value;
}

byte NUM_TYPE;
byte LIST_TYPE;
byte ARG_TYPE;
byte SET_TYPE;
byte RET_TYPE;
byte IF_TYPE;
byte UNLESS_TYPE;
byte WHILE_TYPE;
byte VOID_TYPE;

fn(i, parse_call, ctx con) {
    var call_name = parse_ref_str(con, mem);

    // if (call_name == null) error(con, "Expected target name when parsing call...");
    if (call_name == -1) return null;
    if (call_name == 0) { // index zero is fn
        con->current -= 2;
        return null;
    }
    debug(call);

    i val = ret(struct i);

    val->type            = tcall;
    val->value.call.name = call_name;
    val->value.call.args = (void *) ret(i, 0);

    parse_null(con);

    var last = (i) 0x1;
    while (last != null) {
        parse_null(con);

        last = parse_expr(con, mem);
        if (last != null) push(val->value.call.args, last, mem);

        parse_null(con);
    }

    if (val->value.call.name == NUM_TYPE || val->value.call.name == LIST_TYPE) {
        if (val->value.call.args->size != 2 && val->value.call.args->size != 1) {
            error(con, "The functions num and list require one or two arguments!");
        }

        var first = val->value.call.args->array[ 0 ];

        if (first->type != tref) {
            error(con, "First argument (type) of num or list should always be a keyword!");
        }
    }

    return val;
}

fn(i, parse_block, ctx con) {
    if (cur() != '[') return null;
    debug(block);
    skip();

    i val = ret(struct i);

    val->type        = tcode;
    val->value.block = (void *) ret(i, 0);

    byte start        = 1;
    byte removed_semi = 0;

    parse_null(con);

    while (cur() != ']') {
        parse_null(con);

        if (!start && !removed_semi) {
            if (cur() != ';') error(con, "Expected semicolon after action inside block...");

            skip();
            parse_null(con);
        }

        removed_semi = 0;

        var ex = parse_call(con, mem);
        if (ex != null) push(val->value.block, ex, mem);
        else if (cur() != ']') { error(con, "Expected expression inside block..."); }

        parse_null(con);

        if (cur() == ';') {
            skip();
            removed_semi = 1;
        }

        parse_null(con);
        start = 0;
    }

    skip();

    if (val->value.block->size == 0) error(con, "Empty blocks are not supported!");

    return val;
}

fn(i, parse_fn, ctx con) {
    ground();
    debug(fn);

    var prev    = con->current;
    var keyword = parse_ref_str(con, mem);
    if (keyword != 0) {
        release();
        con->current = prev;
        return null;
    }
    parse_null(con);

    var type = parse_ref_str(con, mem);
    if (type != NUM_TYPE && type != LIST_TYPE && type != VOID_TYPE) {
        printf("%li\n", type);
        error(con, "Expected function type when parsing function...");
    }
    parse_null(con);

    var fn_name = parse_ref_str(con, mem);
    if (fn_name == -1) error(con, "Expected function name when parsing function...");
    if (fn_name == NUM_TYPE || fn_name == LIST_TYPE || fn_name == VOID_TYPE)
        error(con, "Function name cannot be reserved words 'num', 'list', 'void'!");

    parse_null(con);

    var code = parse_block(con, mem);
    if (code == null) error(con, "Expected body when parsing function...");

    parse_null(con);

    if (cur() != ';') error(con, "Expected semicolon at the end of a function declaration...");

    i val = ret(struct i);

    val->type           = tfn;
    val->value.fn.type  = type == NUM_TYPE    ? exp_num
                          : type == LIST_TYPE ? exp_list
                          : type == VOID_TYPE
                              ? exp_void
                              : ({
                                   error(con, "Function type must be 'num', 'list' or 'void'!");
                                   exp_void;
                               });
    val->value.fn.name  = fn_name;
    val->value.fn.block = (void *) code;

    val->value.fn.args = (void *) ret(symbol, 0);
    var my_types       = ret(char, 0);

    for (i32 i = 0; i < code->value.block->size; i++) {
        var current = code->value.block->array[ i ];
        if (current->type == tcall && current->value.call.name == ARG_TYPE) {
            var args = current->value.call.args;

            if (args->size != 2) {
                error(
                    con,
                    "A function argument call must have exactly two arguments (its type and "
                    "name)!");
            }

            var name = args->array[ 1 ];
            if (name->type != tref)
                error(con, "A function argument's name must be a reference (keyword)!");

            var type = args->array[ 0 ];
            if (type->type != tref)
                error(con, "A function argument's type must be a reference (type)!");

            push(val->value.fn.args, name->value.ref, mem);
            push(my_types, (type->value.ref == NUM_TYPE ? -1 : 1), mem);

            if (val->value.fn.args->size > 256) {
                error(
                    con,
                    "A function cannot accept more than 256 arguments! What are you even doing "
                    "with that many arguments?? Not writing readable code, that's for sure!");
            }

            // WARN: Block values may be null!
            // code->value.block->array[ i ] = null;
        }
    }

    var entry = (struct Entry) { .type     = etfn,
                                 .value.fn = { .arg_count   = val->value.fn.args->size,
                                               .name        = fn_name,
                                               .implemented = 0,
                                               .types       = my_types->array,
                                               .ret         = val->value.fn.type } };
    push(con->literals, entry, mem);

    val->value.fn.entry = con->literals->size - 1;

    release();
    return val;
}

fn(i, parse_action, ctx con) {
    // debug(action);
    var value = parse_fn(con, mem);
    if (value == null) value = parse_call(con, mem);

    return value;
}

typedef A(symbol) * symbolic;

byte suggestions = 1;

fn(exp_type, __validate, i val, symbolic symbols, ctx context, exp_type top_type) {
    if (val == null) return exp_void;

#ifdef enable_debug
    print_i(val, 0, context, 0);

    for (i32 i = 0; i < symbols->size; i++) printf("\nsym %li\n", symbols->array[ i ]);
    printf("\n\n");
#endif

    if (val->type == tref) {
        // Search for symbol in symbolic tree

        for (i32 i = 0; i < symbols->size; i++) {
            if (labs(symbols->array[ i ]) == labs(val->value.ref)) {
                var magic      = symbols->array[ i ] < 0 ? exp_num : exp_list;
                val->value.ref = magic * labs(val->value.ref);
                return magic;
            }
        }

        print_i(val, 0, context, 0);
        printf("\n\nReference index: %li\n", val->value.ref);
        error(analyzer_error, "Reference keyword not found!");
        return exp_void;
    } else if (val->type == tchr) {
        // Pass. No checks here.
        return exp_num;
    } else if (val->type == tstr) {
        if (suggestions) {
            var index = context->literals->array[ val->value.str_id ];
            if (index.references > 5) {
                printf(
                    "<Overlord> Hey, there are way too many references to the string \"%s\". You "
                    "should probably make it a constant!\n",
                    index.value.str.array);
            }
        }

        return exp_list;
    } else if (val->type == tnum) {
        if (suggestions) {
            var index = context->literals->array[ val->value.str_id ];
            if (index.references > 5 && (abs((int) index.value.num) > 2)) {
                printf(
                    "<Overlord> Hey, there are way too many references to the number %li. You "
                    "should probably make it a constant!\n",
                    index.value.num);
                context->literals->array[ val->value.str_id ].references = 0;
            }
        }

        return exp_num;
    } else if (val->type == tfn) {
        if (context->literals->array[ val->value.fn.entry ].value.fn.implemented) {
            print_i(val, 0, context, 0);
            printf(
                "\n\nReferenced function: '%s'\n", context->symbols->array[ val->value.fn.entry ]);
            error(analyzer_error, "Duplicate function definition!");
        }

        context->literals->array[ val->value.fn.entry ].value.fn.implemented = 1;

        var clone = (symbolic) copy(symbols, mem);

        // for (i32 i = 0; i < val->value.fn.args->size; i++) {
        //     push(clone, val->value.fn.args->array[ i ], mem);
        // }

        __validate(val->value.fn.block, clone, context, val->value.fn.type, mem);

        return exp_void;
    } else if (val->type == tcall) {
        struct Entry target = { 0 };
        byte         found  = 0;

        for (i32 i = 0; i < context->literals->size; i++) {
            target = context->literals->array[ i ];
            if (target.type == etfn && target.value.fn.name == val->value.call.name) {
                found = 1;
                break;
            }
        }

        if (!found) {
            print_i(val, 0, context, 0);
            printf("\n\nReferenced call: '%s'\n", context->symbols->array[ val->value.call.name ]);
            error(analyzer_error, "Function referenced in call was not found!");
        }

        if (target.value.fn.arg_count != val->value.call.args->size) {
            print_i(val, 0, context, 0);
            printf(
                "\n\nReferenced call: '%s' expected %i arguments but got %li\n",
                context->symbols->array[ val->value.call.name ],
                target.value.fn.arg_count,
                val->value.call.args->size);
            error(analyzer_error, "Mismatched argument count!");
        }

        if (val->value.call.name == NUM_TYPE) {
            for (i32 i = 0; i < symbols->size; i++) {
                if (labs(symbols->array[ i ]) != labs(val->value.call.args->array[ 0 ]->value.ref))
                    continue;
                print_i(val, 0, context, 0);
                printf(
                    "\n\nReferenced declaration: '%s'\n",
                    context->symbols->array[ val->value.call.name ]);
                error(analyzer_error, "Symbol already declared at this scope or higher!");
            }

            push(symbols, labs(val->value.call.args->array[ 0 ]->value.ref) * -1, mem);
            __validate(val->value.call.args->array[ 1 ], symbols, context, top_type, mem);
            return -1;
        } else if (val->value.call.name == LIST_TYPE) {
            for (i32 i = 0; i < symbols->size; i++) {
                if (labs(symbols->array[ i ]) != labs(val->value.call.args->array[ 0 ]->value.ref))
                    continue;
                print_i(val, 0, context, 0);
                printf(
                    "\n\nReferenced declaration: '%s'\n",
                    context->symbols->array[ val->value.call.name ]);
                error(analyzer_error, "Symbol already declared at this scope or higher!");
            }

            push(symbols, val->value.call.args->array[ 0 ]->value.ref, mem);
            return 1;
        } else if (val->value.call.name == ARG_TYPE) {
            for (i32 i = 0; i < symbols->size; i++) {
                if (labs(symbols->array[ i ]) != labs(val->value.call.args->array[ 0 ]->value.ref))
                    continue;
                print_i(val, 0, context, 0);
                printf(
                    "\n\nReferenced declaration: '%s'\n",
                    context->symbols->array[ val->value.call.name ]);
                error(analyzer_error, "Symbol already declared at this scope or higher!");
            }

            var type  = val->value.call.args->array[ 0 ]->value.ref;
            var magic = (type == NUM_TYPE ? -1 : 1);
            push(symbols, labs(val->value.call.args->array[ 1 ]->value.ref) * magic, mem);
            return magic;
        } else if (val->value.call.name == SET_TYPE) {
            var name = val->value.call.args->array[ 0 ]->value.ref;

            // name type must be ref
            if (val->value.call.args->array[ 0 ]->type != tref) {
                print_i(val, 0, context, 0);
                printf("\n");
                error(analyzer_error, "Set command requires a name as its first argument!");
            }

            var exists = 0;
            var type   = 0;
            for (i32 i = 0; i < symbols->size; i++) {
                if (labs(symbols->array[ i ]) == labs(name)) {
                    exists = 1;
                    type   = symbols->array[ i ] < 0 ? -1 : 1;
                    break;
                }
            }

            var inferred
                = __validate(val->value.call.args->array[ 1 ], symbols, context, top_type, mem);
            if (!exists) { // infer type from value
                type = inferred;
            }

            if (inferred != type && exists) {
                print_i(val, 0, context, 0);
                printf("\n");
                error(analyzer_error, "Set command's target type must match the variable type!");
            }

            push(symbols, labs(val->value.call.args->array[ 0 ]->value.ref) * type, mem);

            return 0;
        } else if (val->value.call.name == RET_TYPE) {
            if (val->value.call.args->size == 0) {
                if (top_type == exp_void) return 0;
                else {
                    print_i(val, 0, context, 0);
                    printf("\n");
                    error(
                        analyzer_error,
                        "A return command in a non-void function must have a return value!");
                }
            } else if (top_type == exp_void) {
                print_i(val, 0, context, 0);
                printf("\n");
                error(analyzer_error, "A return command in a void function takes zero arguments!");
            }

            var value = val->value.call.args->array[ 0 ];
            var type  = __validate(value, symbols, context, top_type, mem);

            if (type != top_type) {
                print_i(val, 0, context, 0);
                printf("\n");
                error(analyzer_error, "Return command's argument must match the function's type!");
            }

            return exp_void;
        }

        for (i32 i = 0; i < val->value.call.args->size; i++) {
            var expectation = target.value.fn.types[ i ];

            // printf("%s %li %i %i\n", context->symbols->array[val->value.call.name], i,
            // expectation, reality);

            var reality
                = __validate(val->value.call.args->array[ i ], symbols, context, top_type, mem);
            if (expectation == 0) continue;
            if (expectation == 2) { // Code block
                if (val->value.call.args->array[ i ]->type != tcode) {
                    print_i(val, 0, context, 0);
                    printf(
                        "\n\nReferenced call: '%s' on argument %li:\n    ",
                        context->symbols->array[ val->value.call.name ],
                        i);
                    print_i(val->value.call.args->array[ i ], 0, context, 0);
                    printf("\n\n");
                    error(analyzer_error, "Expected code block!");
                }
            } else {
                if (expectation != reality) {
                    print_i(val, 0, context, 0);
                    printf(
                        "\n\nReferenced call: '%s' on argument %li:\n    ",
                        context->symbols->array[ val->value.call.name ],
                        i);
                    print_i(val->value.call.args->array[ i ], 0, context, 0);
                    printf("\n\n");
                    if (expectation == 1) error(analyzer_error, "Expected list but got number!");
                    else
                        error(analyzer_error, "Expected number but got list!");
                }
            }
        }

        return target.value.fn.ret;
    } else if (val->type == tcode) {
        var cloned_syms = copy(symbols, mem);

        val->value.block->symbols = cloned_syms;

        exp_type output = exp_void;
        for (i32 i = 0; i < val->value.block->size; i++) {
            output = __validate(val->value.block->array[ i ], cloned_syms, context, top_type, mem);
        }

        return output;
    } else if (val->type == tembd) {
        // Pass. There's no checks to be done.
        return exp_any;
    } else {
        printf("Type index: %i\n", val->type);
        error(analyzer_error, "Unknown type in validation!");
        return exp_void;
    }
}

fn(void, validate, A(i) * terms, ctx context) {
    // ground();

    var symbols = (symbolic) ret(symbol, 0);

    for (i32 i = 0; i < terms->size; i++) {
        __validate(terms->array[ i ], symbols, context, exp_void, mem);
    }

    // release();
}

struct standard_entry {
    char *name;
    i32   args;
    char  types[ 256 ];
    char  ret;
};

// 0 shows any type (list or int)
// 1 shows list type
// -1 shows number type
// 2 shows block type
// 3 shows void type

const struct standard_entry stdlib[] = {
    // Internals
    { .name = "ret", .args = 1, .types = { 0 }, .ret = 3 },       //
    { .name = "arg", .args = 2, .types = { 0, 0 }, .ret = 3 },    //
    { .name = "if", .args = 2, .types = { 0, 2 }, .ret = 3 },     //
    { .name = "unless", .args = 2, .types = { 0, 2 }, .ret = 3 }, //
    { .name = "while", .args = 2, .types = { 0, 2 }, .ret = 3 },  //
    // I/O functions
    { .name = "dump", .args = 1, .types = { 0 }, .ret = 3 },   //
    { .name = "puts", .args = 1, .types = { 1 }, .ret = 1 },   //
    { .name = "putc", .args = 1, .types = { -1 }, .ret = -1 }, //
    { .name = "log", .args = 1, .types = { 0 }, .ret = 3 },    //
    { .name = "print", .args = 1, .types = { 1 }, .ret = 3 },  //
    // Arithmetic and logic
    // TODO: Add higher number arithmetics
    { .name = "add", .args = 2, .types = { -1, -1 }, .ret = -1 },
    { .name = "sub", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "mul", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "div", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "mod", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "and", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "or", .args = 2, .types = { -1, -1 }, .ret = -1 },     //
    { .name = "not", .args = 1, .types = { -1 }, .ret = -1 },        //
    { .name = "dec", .args = 1, .types = { -1 }, .ret = -1 },        //
    { .name = "cmp_gt", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_lt", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_eq", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_ne", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_ge", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_le", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    // Array manipulation
    { .name = "get", .args = 2, .types = { 1, -1 }, .ret = -1 }, // get(list, index): num
    { .name = "push", .args = 2, .types = { 1, -1 }, .ret = 1 }, // push(list, value): list
    { .name = "len", .args = 1, .types = { 1 }, .ret = -1 },     //
    { .name = "num", .args = 2, .types = { 0 }, .ret = -1 },     //
    { .name = "list", .args = 1, .types = { 0 }, .ret = 1 },     //
    { .name = "set", .args = 2, .types = { 0, 0 }, .ret = 3 },   //
    { .name = "len", .args = 1, .types = { 1 }, .ret = -1 },     //
    // Threading
    { .name = "spawn", .args = 1, .types = { 2 }, .ret = -1 },   // fork(): id
    { .name = "send", .args = 2, .types = { -1, 1 }, .ret = 3 }, // send(thread, value): void
    { .name = "fetch", .args = 0, .types = {}, .ret = 1 }        //
};
const var stdlib_size = (sizeof(stdlib) / sizeof(stdlib[ 0 ]));

fn(char *, ltoa, i32 value) {
    ground();

    var output = new (char, 0);
    while (value >= 10) {
        push(output, (value % 10) + '0', scratch);
        value /= 10;
    }
    push(output, value + '0', scratch);

    var real_output = ret(char, 0);
    for (i32 i = output->size - 1; i >= 0; i--) push(real_output, output->array[ i ], mem);

    release();
    return real_output->array;
}

typedef i32 iaddress;
typedef i64 complexsym;

typedef struct {
    i32  size;
    i32  unit;
    i32 *array;
} *IR_SCOPE_ARRAY;
typedef A(IR_SCOPE_ARRAY) * IR_SCOPE;

typedef struct {
    // TODO: Add more features so that bulit-in functions can be represented in the IR.
    enum {
        iruseless,
        irpop,
        irpush,
        ircall,
        irret,
        irjmp0,
        irjmpn0,
        irnop,
        irconst,
        iraddr,
        irset
    } op;
    union {
        struct IR_INST_POP {
        } ipop; // Pops value from stack and places it in TMP
        struct IR_INST_PUSH {
        } ipush; // Push value in TMP to stack
        struct IR_INST_CALL {
            iaddress ref;
        } icall; // Call address and store current PC + 1 in the stack to return.
        struct IR_INST_RET {
        } ret; // Pop the top value in call stack and jump to it.
        struct IR_INST_JMP0 {
            iaddress ref;
        } ijmp0; // Jump to address if TMP is 0
        struct IR_INST_JMPN0 {
            iaddress ref;
        } ijmpn0; // Jump to address if TMP is not 0
        struct IR_INST_NOP {
        } nop; // Do nothing
        struct IR_INST_CONST {
            i32 value;
        } iconstant; // Set TMP to value
        struct IR_INST_ADDR {
            complexsym value;
            complexsym actuator;
        } iaddr; // Value > 0 ? Set(TMP, [value]) : Set(TMP, value)
        struct IR_INST_SET {
            complexsym dest;
        } iset; // Set [dest] to TMP
    } data;
    IR_SCOPE scope;
} IR_INST;

const IR_INST push_stack = { .op = irpush, .data = {} };
const IR_INST nop        = { .op = irnop, .data.nop = {} };

typedef struct {
    enum { ir_lnum, ir_lstr } type;
    union {
        i32    number;
        string string;
    } value;
} IR_LITERAL;

typedef struct {
    symbol name;
    A(IR_INST) * body;
} IR_FUNCTION;

typedef A(IR_INST) * INST_LIST;

typedef struct {
    A(IR_FUNCTION) * segments;
    A(IR_LITERAL) * literals;

    i32 inst_count;
    i32 main_segment;
    i32 global_name_scope_id;
} IR;

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

void print_inst(IR_INST inst, IR scope, ctx context) {
    ground();

    switch (inst.op) {
        case irpop: printf("POP\n"); break;
        case irpush: printf("PUSH\n"); break;
        case ircall:;
            var my_name = scope.segments->array[ inst.data.icall.ref ].name;
            printf(
                red("CALL") yellow(" __0x%lX ") POS_ALIGN "; %s\n",
                my_name < 0 ? -my_name : inst.data.icall.ref,
                my_name < 0 ? "block" : context->symbols->array[ my_name ]);
            break;
        case irret: printf("RET\n"); break;
        case irjmp0: printf(red("JMP0 ") yellow("__0x%lX\n"), inst.data.ijmp0.ref); break;
        case irjmpn0: printf(red("JMPN0") yellow(" __0x%lX\n"), inst.data.ijmpn0.ref); break;
        case irconst: printf(magenta("CONST ") green("%li\n"), inst.data.iconstant.value); break;
        case iraddr: {
            var abs2     = llabs(inst.data.iaddr.value);
            var noderef  = inst.data.iaddr.value < 0;
            var subindex = (abs2 << 32) >> 32;
            var topindex = abs2 >> 32;
            var is_str   = inst.data.iaddr.actuator == 1;
            var abs      = is_str ? 1 : inst.scope->array[ topindex ]->array[ subindex ];

            var name = is_str    ? context->literals->array[ abs2 ].value.str.array
                       : noderef ? context->symbols->array[ abs ]
                                 : context->symbols->array[ abs ];

            /*
                abs > context->symbols->size ? "unknown"
                : noderef                    ? !is_str ? context->symbols->array[ abs ]
                                                       : (context->literals->array[ abs2
               ].value.str.array) : context->symbols->array[ abs ]
            */

            printf(
                bright_blue("ADDR") " "
                                    "%s" yellow("0x%lX_%llX") "%s" POS_ALIGN "; %s%s%s\n",
                noderef ? "" : "[",
                is_str ? 0 : inst.scope->array[ topindex ]->array[ 0 ],
                is_str ? abs2 : subindex,
                noderef ? "" : "]",
                is_str    ? "\""
                : noderef ? ""
                          : "[",
                name,
                is_str    ? "\""
                : noderef ? ""
                          : "]");
            break;
        }
        case irset:;
            var abs2     = llabs(inst.data.iset.dest);
            var subindex = (abs2 << 32) >> 32;
            var topindex = abs2 >> 32;
            var abs      = inst.scope->array[ topindex ]->array[ subindex ];
            printf(
                green("SET") " " yellow("0x%lX_%llx") POS_ALIGN "; %s\n",
                inst.scope->array[ topindex ]->array[ 0 ],
                subindex,
                abs > context->symbols->size ? "unknown" : context->symbols->array[ abs ]);
        case irnop: /* printf("NOP\n"); break; */
        case iruseless: printf("\r\r\r\r"); break;
    }

    printf(C_RESET);
    release();
}

fn(void, tourist, IR scope, ctx context) {
    ground();

    i32 skips = 0;

    printf("== Intermediate Representation ==\n\n");
    for (i32 i = scope.segments->size; i >= 0; i--) {
        var cur = scope.segments->array[ i ];

        if (*(i32 *) &cur == 0) continue;
        if (cur.body->size == 0) continue;

        if (cur.name == 1) printf(bold_blue("main") ":\n");
        else
            printf(bold_blue("__0x%lX") ":\n", i);
        for (i32 j = 0; j < cur.body->size; j++) {
            var inst = cur.body->array[ j ];
            if (inst.op == iruseless || inst.op == irnop) {
                skips++;
                continue;
            }
            printf("    ");
            print_inst(inst, scope, context);
        }
        printf("\n");
    }

    printf(
        "=== Tourist's Take ===\n - %li instruction%s\n - %li literal%s\n - %li segment%s (%li "
        "total, %li builtin)\n\n",
        scope.inst_count - skips,
        scope.inst_count - skips == 1 ? "" : "s",
        scope.literals->size,
        scope.literals->size == 1 ? "" : "s",
        scope.segments->size - stdlib_size * 2,
        (scope.segments->size - stdlib_size * 2) == 1 ? "" : "s",
        scope.segments->size - stdlib_size,
        stdlib_size);

    printf("== Literals ==\n");
    for (i32 i = 0; i < scope.literals->size; i++) {
        var item = scope.literals->array[ i ];
        printf(
            " - Index(%li) Type(%s) | Value(%s)\n",
            i,
            item.type == ir_lnum ? "number" : "string",
            item.type == ir_lstr ? item.value.string.array : ltoa(item.value.number, scratch));
    }

    release();
}

fn(void, represent, i term, IR *scope, IR_SCOPE name_scope, INST_LIST cur, ctx context) {
    if (term == null) return;

    scope->inst_count++;

    var result   = (IR_INST) { .op = iruseless };
    result.scope = name_scope;

    if (term->type == tchr) {
        result.op                   = irconst;
        result.data.iconstant.value = term->value.chr;
    } else if (term->type == tnum) {
        result.op                   = irconst;
        result.data.iconstant.value = context->literals->array[ term->value.num_id ].value.num;
    } else if (term->type == tstr) { // what to do here?
        result.op                  = iraddr;
        result.data.iaddr.value    = -term->value.str_id;
        result.data.iaddr.actuator = 1;
    } else if (term->type == tref) {
        result.op  = iraddr;
        var target = labs(term->value.ref);

        for (i32 i = name_scope->size - 1; i >= 0; i--) {
            var scope = name_scope->array[ i ];

            for (i32 j = 1; j < scope->size; j++) {
                var cur = scope->array[ j ];
                if (cur != target) continue;

                result.data.iaddr.value = ((i64) i << 32) + (i64) j;
                // printf("Is(%i) %llX\n", context->literals->array[term->value.ref].type,
                // result.data.iaddr.value);
                result.data.iaddr.value *= term->value.ref > 0 ? -1 : 1;
                // printf("%s %li\n", context->symbols->array[target], term->value.ref);
                goto END;
            }
        }

        print_i(term, 0, context, 0);
        printf("\n\n");
        error(ir_error, "Invalid or unknown reference!");
    } else if (term->type == tcall) {
        var name = term->value.call.name;
        // TODO: Custom native handling (eg. set, ret)

        if (name == NUM_TYPE) {
            result.op  = irset;
            var target = term->value.call.args->array[ 0 ]->value.ref;

            push(name_scope->array[ name_scope->size - 1 ], target, mem);
            result.data.iset.dest = (((i64) name_scope->size - 1) << 32)
                                    + (i64) (name_scope->array[ name_scope->size - 1 ]->size - 1);

            represent(term->value.call.args->array[ 1 ], scope, name_scope, cur, context, mem);

            goto END;
        } else if (name == LIST_TYPE) {
            // Initialize a list
            var target = term->value.call.args->array[ 0 ]->value.ref;
            push(name_scope->array[ name_scope->size - 1 ], target, mem);

            represent(term->value.call.args->array[ 0 ], scope, name_scope, cur, context, mem);

            result.op = ircall;

            var real_name = scope->segments->array[ name ].name;

            for (i32 i = 0; i < scope->segments->size; i++) {
                if (scope->segments->array[ i ].name != name) continue;
                real_name = i;
            }

            push(cur, push_stack, mem);

            result.data.icall.ref = real_name;

            goto END;
        } else if (name == IF_TYPE) {
            var seg_name          = scope->segments->size;
            var me                = (IR_FUNCTION) { .name = -seg_name, .body = (void *) cur };
            var new_scope         = (IR_SCOPE_ARRAY) ret(i32, 1);
            new_scope->array[ 0 ] = scope->global_name_scope_id++;
            if (parser_optimizations <= OPT_BASIC) {
                me.body = (void *) ret(IR_INST, 0);
                push(scope->segments, me, mem);
            }

            var my_scope = (IR_SCOPE) copy(name_scope, mem);
            push(my_scope, new_scope, mem);
            represent(term->value.call.args->array[ 0 ], scope, my_scope, cur, context, mem);

            result.op              = irjmpn0;
            result.data.ijmpn0.ref = seg_name;

            represent(
                term->value.call.args->array[ 1 ],
                scope,
                name_scope,
                (void *) me.body,
                context,
                mem);

            // cur->size--;

            goto END;
        } else if (name == UNLESS_TYPE) {
            var seg_name          = scope->segments->size;
            var me                = (IR_FUNCTION) { .name = -seg_name, .body = (void *) cur };
            var new_scope         = (IR_SCOPE_ARRAY) ret(i32, 1);
            new_scope->array[ 0 ] = scope->global_name_scope_id++;
            if (parser_optimizations <= OPT_BASIC) {
                me.body = (void *) ret(IR_INST, 0);
                push(scope->segments, me, mem);
            }

            var my_scope = (IR_SCOPE) copy(name_scope, mem);
            push(my_scope, new_scope, mem);
            represent(term->value.call.args->array[ 0 ], scope, my_scope, cur, context, mem);

            result.op             = irjmp0;
            result.data.ijmp0.ref = seg_name;

            represent(
                term->value.call.args->array[ 1 ],
                scope,
                name_scope,
                (void *) me.body,
                context,
                mem);

            goto END;
        } else if (name == WHILE_TYPE) {
            var seg_name          = scope->segments->size;
            var me                = (IR_FUNCTION) { .name = -seg_name, .body = (void *) cur };
            var new_scope         = (IR_SCOPE_ARRAY) ret(i32, 1);
            new_scope->array[ 0 ] = scope->global_name_scope_id++;
            if (parser_optimizations <= OPT_BASIC) {
                me.body = (void *) ret(IR_INST, 0);
                push(scope->segments, me, mem);
            }

            var my_scope = (IR_SCOPE) copy(name_scope, mem);
            push(my_scope, new_scope, mem);
            represent(term->value.call.args->array[ 0 ], scope, my_scope, cur, context, mem);

            result.op              = irjmpn0;
            result.data.ijmpn0.ref = seg_name;

            represent(
                term->value.call.args->array[ 1 ],
                scope,
                name_scope,
                (void *) me.body,
                context,
                mem);

            represent(
                term->value.call.args->array[ 0 ],
                scope,
                name_scope,
                (void *) me.body,
                context,
                mem);

            push(me.body, result, mem);

            goto END;
        } else if (name == SET_TYPE) {
            represent(term->value.call.args->array[ 1 ], scope, name_scope, cur, context, mem);

            var target   = term->value.call.args->array[ 0 ]->value.ref;
            var resolved = (i64) -1;

            for (i32 i = name_scope->size - 1; i >= 0; i--) {
                var scope = name_scope->array[ i ];

                for (i32 j = 0; j < scope->size; j++) {
                    var cur = scope->array[ j ];
                    if (labs(cur) != labs(target)) continue;

                    resolved = (((i64) i) << 32) + (i64) j;
                    goto SET_FINISH;
                }
            }

            push(name_scope->array[ name_scope->size - 1 ], target, mem);
            resolved = (((i64) name_scope->size - 1) << 32)
                       + (i64) (name_scope->array[ name_scope->size - 1 ]->size - 1);

        SET_FINISH:

            result.op             = irset;
            result.data.iset.dest = resolved;

            goto END;
        } else if (name == ARG_TYPE) {
            // FIXME
            var pop_stack = (IR_INST) { .op = irpop, .data.ipop = {} };

            push(cur, pop_stack, mem);

            var target   = term->value.call.args->array[ 1 ]->value.ref;
            var resolved = (i64) -1;

            for (i32 i = name_scope->size - 1; i >= 0; i--) {
                var scope = name_scope->array[ i ];

                for (i32 j = 0; j < scope->size; j++) {
                    var cur = scope->array[ j ];
                    if (labs(cur) != labs(target)) continue;

                    resolved = (((i64) i) << 32) + (i64) j;
                    goto ARG_FINISH;
                }
            }

            // error here

        ARG_FINISH:

            result.op             = irset;
            result.data.iset.dest = resolved;

            goto END;
        } else if(name == RET_TYPE) {
            for(i32 i = 0; i < term->value.call.args->size; i++) {
                represent(term->value.call.args->array[i], scope, name_scope, cur, context, mem);
            }

            result.op = irret;

            goto END;
        }

        var push_stack = (IR_INST) { .op = irpush, .data.ipush = {} };

        for (i32 i = term->value.call.args->size - 1; i >= 0; i--) {
            represent(term->value.call.args->array[ i ], scope, name_scope, cur, context, mem);
            push(cur, push_stack, mem);
        }

        result.op = ircall;

        var real_name = scope->segments->array[ name ].name;

        for (i32 i = 0; i < scope->segments->size; i++) {
            if (scope->segments->array[ i ].name != name) continue;
            real_name = i;
        }

        result.data.icall.ref = real_name;
    } else if (term->type == tcode) {
        if (term->value.block->size >= 16 || parser_optimizations > OPT_BASIC) {
            var seg_name = scope->segments->size;
            var me       = (IR_FUNCTION) { .name = -seg_name, .body = (void *) ret(IR_INST, 0) };
            push(scope->segments, me, mem);

            var new_scope         = (IR_SCOPE_ARRAY) ret(i32, 1);
            new_scope->array[ 0 ] = scope->global_name_scope_id++;
            var my_scope          = (IR_SCOPE) copy(name_scope, mem);
            push(my_scope, new_scope, mem);

            for (i32 i = 0; i < term->value.block->size; i++) {
                represent(
                    term->value.block->array[ i ],
                    scope,
                    my_scope,
                    (INST_LIST) me.body,
                    context,
                    mem);
            }

            result.op             = ircall;
            result.data.icall.ref = seg_name;
        } else {
            for (i32 i = 0; i < term->value.block->size; i++) {
                represent(term->value.block->array[ i ], scope, name_scope, cur, context, mem);
            }

            // result.op = irnop;
        }
    } else if (term->type == tfn) {
        var my_segment
            = (IR_FUNCTION) { .name = term->value.fn.name, .body = (void *) ret(IR_INST, 0) };
        push(scope->segments, my_segment, mem);

        var new_scope         = (IR_SCOPE_ARRAY) ret(i32, 1);
        new_scope->array[ 0 ] = scope->global_name_scope_id++;

        for (i32 i = 0; i < term->value.fn.args->size; i++) {
            push(new_scope, term->value.fn.args->array[ i ], mem);
        }

        var my_scope = (IR_SCOPE) copy(name_scope, mem);
        push(my_scope, new_scope, mem);

        for (i32 i = 0; i < term->value.fn.block->value.block->size; i++) {
            represent(
                term->value.fn.block->value.block->array[ i ],
                scope,
                my_scope,
                (INST_LIST) my_segment.body,
                context,
                mem);
        }
    } else if (term->type == tembd) {
        print_i(term, 0, context, 0);
        printf("\n\n");
        error(ir_error, "Embedded direct code is not supported!");
    } else {
        print_i(term, 0, context, 0);
        printf("\n\n");
        error(ir_error, "Unknown AST node type!");
    }

END:
    // printf("```\n");

    // print_i(term, 0, context, 0);
    // printf("\n");

    // print_inst(result, context);

    // printf("```\n\n\n");

    result.scope = name_scope;

    push(cur, result, mem);
    // if (result.op == irconst) push(cur, push_stack, mem);
}

fn(IR, emit, A(i) * term, ctx context) {
    var output = (IR) { .segments             = (void *) ret(IR_FUNCTION, 0),
                        .literals             = (void *) ret(IR_LITERAL, 0),
                        .inst_count           = 0,
                        .global_name_scope_id = 0 };

    var main_segment = (IR_FUNCTION) { .name = 1, .body = (void *) ret(IR_INST, 0) };

    push(output.segments, main_segment, mem);

    for (i32 t_p = 0; t_p < stdlib_size; t_p++) { push(output.segments, (IR_FUNCTION) { 0 }, mem); }

    // clone literals and functions into the output
    for (i32 i = 0; i < context->literals->size; i++) {
        var item = context->literals->array[ i ];

        if (item.type == etnum) {
            var ref = (IR_LITERAL) { 0 };

            ref.type         = ir_lnum;
            ref.value.number = item.value.num;

            push(output.literals, ref, mem);
        } else if (item.type == etstr) {
            var ref = (IR_LITERAL) { 0 };

            ref.type         = ir_lstr;
            ref.value.string = item.value.str;

            push(output.literals, ref, mem);
        } else if (item.type == etfn) {
            var func
                = (IR_FUNCTION) { .name = item.value.fn.name, .body = (void *) ret(IR_INST, 0) };

            push(output.segments, func, mem);
        } else {
            error(ir_error, "Unknown literal type!");
        }
    }

    // var push_stack = (IR_INST) { .op = irpush, .data.ipush = {} };

    var scopes             = ret(IR_SCOPE_ARRAY, 0);
    var main_scope         = (IR_SCOPE_ARRAY) ret(i32, 1);
    main_scope->array[ 0 ] = output.global_name_scope_id++;

    push(scopes, main_scope, mem);

    // Compute normal terms
    for (i32 i = 0; i < term->size; i++) {
        var item = term->array[ i ];
        if (item == null) continue;

        represent(item, &output, (void *) scopes, (void *) main_segment.body, context, mem);
    }

    return output;
}

typedef A(byte)* bytecode;

int get_eax_value() {
    int eax_value;

    // Inline assembly to move the value in EAX into the variable 'eax_value'
    __asm__ (
        "mov %%eax, %0;"  // Move the value in EAX into the C variable eax_value
        : "=r" (eax_value) // Output: store the value of EAX into eax_value
        :                  // No input operands
        : "%eax"           // Clobber: We are modifying the EAX register
    );

    return eax_value;
}

// [ ] pop(mov [cur_stack], %rex);
// [ ] push(push_stack %rex);
// [ ] call(complicated...);
// [ ] ret(complicated...);
// [ ] jmp0(cmp %rex; jmp0 [addr]);
// [ ] jmpn0(reverse of jmp0)
// [ ] addrI(mov %rex, addr);
// [ ] addrD(mov %rex, [addr]);
// [ ] set(mov [addr], %rex);
// [x] const(mov %rex, $value);
// [x] nop(); # Why would I even add this?

// MOV $value, EAX
fn(void, machine_const, i32 value, bytecode target) {
    // Code: c7 45 ec XX XX XX XX
    // The XX XX XX XX contains the reverse of the value.
    push(target, 0xb8, mem);
    // push(target, 0x45, mem);
    // push(target, 0xec, mem);
    push(target, (byte) (value & 0xff), mem);
    push(target, (byte) ((value >> 8) & 0xff), mem);
    push(target, (byte) ((value >> 16) & 0xff), mem);
    push(target, (byte) ((value >> 24) & 0xff), mem);
}

fn(void, machine_addr_immediate, i32 value, bytecode target) {
    // Steps:
    // 1. Figure out ELF
    // 2. Figure out DATA segment
    // 3. Figure out TEXT segment
    // 4. Rewrite the arena allocator in assembly, then hand-assemmble it to x86
    // 5. Utilize that to perform calls and returns
    // 6. Rewrite every standard function in the language in assembly then hand-assemble it to x86
    // This process is REQUIRED for addrI, addrD, jmp0, jmpn0, call, ret, set, pop and push
}

// RET
fn(void, machine_ret, bytecode target) {
    push(target, 0xc3, mem);
}

ctx main_setup(char *filename, Arena *scratch) {
    var con       = new (struct _ctx);
    con->current  = 0;
    con->str      = *read_file(filename, scratch);
    con->literals = (void *) new (struct Entry, 0);

    con->symbols = (void *) new (string, 0);

    push(con->symbols, "fn", scratch);
    push(con->symbols, "void", scratch);
    VOID_TYPE = 1;

    // push stdlib
    for (i32 i = 0; i < sizeof(stdlib) / sizeof(struct standard_entry); i++) {
        push(con->symbols, stdlib[ i ].name, scratch);

        var entry = (struct Entry) { .type     = etfn,
                                     .value.fn = { .name        = con->symbols->size - 1,
                                                   .arg_count   = stdlib[ i ].args,
                                                   .implemented = 1,
                                                   .types       = (char *) stdlib[ i ].types,
                                                   .ret         = stdlib[ i ].ret } };
        push(con->literals, entry, scratch);

        if (!strcmp(stdlib[ i ].name, "list")) LIST_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "num")) NUM_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "arg")) ARG_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "set")) SET_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "ret")) RET_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "if")) IF_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "unless")) UNLESS_TYPE = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "while")) WHILE_TYPE = con->symbols->size - 1;
    }

    return con;
}

A(i) * main_parse(ctx con, Arena *scratch) {
    var terms = new (i, 0);

    parse_null(con);
    while (!eof()) {
        var result = parse_action(con, scratch);
        if (result == null) {
            parse_null(con);
            if (!eof()) { error(con, "Invalid syntax!"); }
        }

        push(terms, result, scratch);
        if (skip() != ';') error(con, "Expected semicolon...");
        parse_null(con);
    }

    return (void *) terms;
}

void print_bytecode(bytecode target) {
    for (size_t i = 0; i < target->size; ++i) {
        printf("%02x ", target->array[i]);
    }
    printf("\n");
}

void __dummy() {}
typedef typeof(__dummy)* mc;

mc get_exec(bytecode target) {
    var executable = mmap(NULL, target->size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                    MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(executable, target->array, target->size);

    return executable;
}

int main(int argc, char **argv) {
    ground();

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[ 0 ]);
        return 1;
    }

    var filename = argv[ 1 ];
    var con      = main_setup(filename, scratch);

    var terms = main_parse(con, scratch);

    // Perform static analysis
    validate((void *) terms, con, scratch);

    // Generate intermediate representation
    var ir = emit((void *) terms, con, scratch);

    // Print AST structure
    for (i32 i = 0; i < terms->size; i++) {
        print_i(terms->array[ i ], 0, con, 0);
        printf("\n");
    }

    printf("\n\n");

    tourist(ir, con, scratch);

    // x86 test
    // var test = (bytecode)new(char, 0);
    // machine_const(30, test, scratch);
    // machine_ret(test, scratch);    

    // var function_pointer = get_exec(test);

    // function_pointer();

    // printf("%i\n", get_eax_value());

    // print_bytecode(test);

    release();
}









// Literally 