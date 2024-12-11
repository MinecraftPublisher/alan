#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef unsigned char byte;
typedef long long     i4;
#define null NULL
char nullchar = '\0';

typedef struct Array {
    i4    size;
    i4    unit;
    byte *array;
} *A;

#define p_cat(a, b) a##b
#define cat(a, b)   p_cat(a, b)
#define A(type)                                                                                    \
    struct {                                                                                       \
        i4    size;                                                                                \
        i4    unit;                                                                                \
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
    i4    size;
    i4    used;
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
        printf("Stack trace (%zd frames):\n", size);
        for (size_t i = 0; i < size; i++) { printf("#%zu: %s\n", i, strings[ i ]); }
        free(strings);
    } else {
        printf("Failed to get stack trace\n");
    }
}

void arena_free(Arena *arena) {
    // printf("Free(%p)\n", arena);
    // dump_stack();
    if (arena->initialized) {
        for (i4 i = 0; i < arena->blocks->size; i++) {
            // printf("Block.Free(%p)\n", arena->blocks->array[ i ].data);
            munmap(arena->blocks->array[ i ].data, arena->blocks->array[ i ].size);
        }
        munmap(arena->blocks->array, arena->blocks->size * arena->blocks->unit);
        munmap(arena->blocks, sizeof(A(Block)));
        arena->initialized = 0;
        arena->blocks      = NULL;
    }
}

#define SIZE_UNIT 8192

void *alloc(Arena *arena, i4 size) {
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

    for (i4 i = 0; i < arena->blocks->size; i++) {
        Block *block = &arena->blocks->array[ i ];
        if (block->size - block->used >= size) {
            void *ptr = &block->data[ block->used ];
            block->used += size;
            // printf("Alloc(%p, %lli, %p)\n", arena, size, ptr);
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

    // printf("Block.New(%p, %lli, %p)\n", arena, size, new_block.data);
    return new_block.data;
}

string str(char *in, i4 len, Arena *arena) {
    var space = (char *) alloc(arena, len);

    for (i4 i = 0; i < len; i++) space[ i ] = in[ i ];
    space[ len ] = 0;

    return (string) { .unit = 1, .size = len, .array = space };
}

fn(A, __new_array, i4 count, i4 unit) {
    A arr      = alloc(mem, sizeof(struct Array));
    arr->size  = count;
    arr->unit  = unit;
    arr->array = alloc(mem, sizeof(byte) * (count + 1) * unit);

    return arr;
}

void extend(A array, i4 count, Arena *mem) {
    var old      = array->array;
    array->array = alloc(mem, sizeof(byte) * (array->size + count) * array->unit);
    memcpy(array->array, old, array->size * array->unit);
    array->size += count;
}

#define push(arr, data, _mem)                                                                      \
    ({                                                                                             \
        extend((void *) arr, 1, _mem);                                                             \
        arr->array[ arr->size - 1 ] = data;                                                        \
    })

typedef i4 symbol;

typedef struct i *i;
struct i {
    enum { tstr, tnum, tcode, tcall, tref, tfn, tchr, tembd } type;
    union {
        i4 str_id;
        i4 num_id;
        A(i) * block;
        struct {
            symbol name;
            A(i) * args;
        } call;
        symbol ref;
        struct {
            symbol name;
            i      block;
            A(symbol) * args;
        } fn;
        char    chr;
        string *direct;
    } value;
};

struct Entry {
    enum { etnum, etstr } type;
    union {
        i4     num;
        string str;
    } value;
};

struct _ctx {
    string str;
    i4     current;
    A(struct Entry) * literals;
    A(char *) * symbols;
};

typedef struct _ctx *ctx;

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

void error(ctx con, char *text) {
    // TODO: Improve error reporting, maybe add stack trace?
    printf("Error: %s\nIndex: %lli\n\n", text, con->current);
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

// #define enable_debug

#ifdef enable_debug
    #define debug(name)                                                                            \
        ({                                                                                         \
            fprintf(stderr, "Parsing " #name " index %lli\n", con->current);                       \
            fprintf(stderr, "```\n%s\n```\n\n", &(con->str.array[ con->current ]));                \
            /*dump_stack();*/                                                                      \
        })
#else
    #define debug(name)
#endif

// ------------------------
// -------- PARSER --------
// ------------------------

fn(symbol, parse_ref_str, ctx con) {
    if (!is_ref_firstchar(cur())) return -1;
    ground();

    var result         = (string *) ret(char, 1);
    result->array[ 0 ] = skip();

    while (is_refchar(cur())) { push(result, skip(), mem); }

    push(result, nullchar, mem);

    symbol index = -1;

    for (i4 i = 0; i < con->symbols->size; i++) {
        if (con->symbols->array[ i ] == null) continue;
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
    var entry = (struct Entry) { .type = etnum, .value.num = atoi(txt->array) };
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

    var entry = (struct Entry) { .type = etstr, .value.str = str(txt->array, txt->size, mem) };
    entry.value.str.array[ entry.value.str.size ] = nullchar;

    push(con->literals, entry, mem);

    val->value.str_id = con->literals->size - 1;

    release();
    return val;
}

fn(string *, trim_str, string input) {
#define whitespace(x)                                                                              \
    ((input.array[ x ] == ' ' || input.array[ x ] == '\t' || input.array[ x ] == '\n'))

    i4 start_i = 0;
    while (whitespace(start_i)) { start_i++; }

    i4 end_i = input.size - 1;
    while ((end_i >= start_i) && whitespace(end_i)) { end_i--; }

    i4 size = end_i - start_i + 1;

    string *new = (void *) ret(char, size);

    i4 o_i = 0;
    for (i4 i = start_i; i <= end_i; i++) { new->array[ o_i++ ] = input.array[ i ]; }

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

    i4 depth = 0;
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
i parse_fn(ctx con, Arena *mem);

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

    return val;
}

fn(i, parse_action, ctx con) {
    // debug(action);
    var value = parse_fn(con, mem);
    if (value == null) value = parse_call(con, mem);

    return value;
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

    // if (val->value.block->size == 0) error(con, "Empty blocks are not supported!");

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

    var fn_name = parse_ref_str(con, mem);
    if (fn_name == -1) error(con, "Expected function name when parsing function...");

    parse_null(con);

    var code = parse_block(con, mem);
    if (code == null) error(con, "Expected body when parsing function...");

    parse_null(con);

    if (cur() != ';') error(con, "Expected semicolon at the end of a function declaration...");

    i val = ret(struct i);

    val->type           = tfn;
    val->value.fn.name  = fn_name;
    val->value.fn.block = (void *) code;

    val->value.fn.args = (void *) ret(i, 0);

    for (i4 i = 0; i < code->value.block->size; i++) {
        var current = code->value.block->array[ i ];
        if (current->type == tcall) {
            var name = current->value.call.name;
            if (name == 1) { // index 1 is arg
                var args = current->value.call.args;

                if (args->size != 1) {
                    error(con, "A function argument must have exactly one argument!");
                }

                var name = args->array[ 0 ];
                if (name->type != tref) error(con, "A function argument must be a reference!");

                push(val->value.fn.args, name->value.ref, mem);

                // WARN: Block values may be null!
                code->value.block->array[ i ] = null;
            }
        }
    }

    release();
    return val;
}

// ------------------------
// ------- PRINTER --------
// ------------------------

fn(void, print_i, i val, i4 indent, ctx con, byte print_sym) {
    if (val == null) printf("\r");
    else if (val->type == tref) {
        if (print_sym) printf("<$%lli>", val->value.ref);
        else
            printf("%s", con->symbols->array[ val->value.ref ]);
    } else if (val->type == tchr)
        printf("'%c'", val->value.chr);
    else if (val->type == tstr) {
        if (print_sym) printf("<\"%lli>", val->value.str_id);
        else
            printf("\"%s\"", con->literals->array[ val->value.str_id ].value.str.array);
    } else if (val->type == tnum) {
        if (print_sym) printf("<#%lli>", val->value.str_id);
        else
            printf("%lli", con->literals->array[ val->value.str_id ].value.num);
    } else if (val->type == tfn) {
        if (print_sym) printf("fn <$%lli> (", val->value.fn.name);
        else
            printf("fn %s (", con->symbols->array[ val->value.fn.name ]);
        for (i4 i = 0; i < val->value.fn.args->size; i++) {
            if (print_sym) printf("<$%lli>", val->value.fn.args->array[ i ]);
            else
                printf("%s", con->symbols->array[ val->value.fn.args->array[ i ] ]);
            if (i + 1 < val->value.fn.args->size) printf(", ");
        }
        printf(") ");

        print_i(val->value.fn.block, indent, con, print_sym, mem);
        printf(";\n");
    } else if (val->type == tcall) {
        if (print_sym) printf("<$%lli> ", val->value.call.name);
        else
            printf("%s ", con->symbols->array[ val->value.call.name ]);
        for (i4 i = 0; i < val->value.call.args->size; i++) {
            print_i(val->value.call.args->array[ i ], indent, con, print_sym, mem);
            if (i + 1 < val->value.call.args->size) printf(" ");
        }
        printf(";");
    } else if (val->type == tcode) {
        printf("[ ");
        if (val->value.block->size > 1) printf("\n");
        for (i4 i = 0; i < val->value.block->size; i++) {
            if (val->value.block->array[ i ] == null) continue;

            if (val->value.block->size > 1) {
                for (i4 sp = 0; sp <= indent; sp++) printf("    ");
            }

            print_i(val->value.block->array[ i ], indent + 1, con, print_sym, mem);
            if (i + 1 < val->value.block->size && val->value.block->size > 1) printf("\n");
        }

        if (val->value.block->size > 1) {
            printf("\n");
            for (i4 sp = 0; sp < indent; sp++) printf("    ");
        } else {
            printf(" ");
        }

        printf("]");
    } else if (val->type == tembd) {
        printf("{{ %s }}", val->value.direct->array);
    } else {
        printf("{unknown - %i}", val->type);
    }
}

fn(void, validate, i term, ctx context) {}

int main() {
    ground();

    var con       = new (struct _ctx);
    con->current  = 0;
    con->str      = *read_file("ts_version/tests/example.al", scratch);
    con->literals = (void *) new (struct Entry, 0);
    con->symbols  = (void *) new (string, 0);

    push(con->symbols, "fn", scratch);
    push(con->symbols, "arg", scratch);
    push(con->symbols, "num", scratch);
    push(con->symbols, "list", scratch);
    push(con->symbols, "if", scratch);
    push(con->symbols, "unless", scratch);
    push(con->symbols, "while", scratch);

    var terms = new (i, 0);

    parse_null(con);
    while (!eof()) {
        var result = parse_action(con, scratch);
        if (result == null) {
            parse_null(con);
            printf("%lli %lli\n", con->current, con->str.size);
            if (!eof()) { error(con, "Invalid syntax!"); }
        }

        push(terms, result, scratch);
        if (skip() != ';') error(con, "Expected semicolon...");
        parse_null(con);
    }

    for (i4 i = 0; i < terms->size; i++) {
        print_i(terms->array[ i ], 0, con, 1, scratch);
        printf("\n");
    }

    release();
}