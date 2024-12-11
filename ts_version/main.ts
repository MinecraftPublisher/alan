import fs from 'fs'
import * as alp from './parser'
import { spawn } from 'child_process'

type $number = ['number', number]
type $keyword = ['keyword', string]
type $string = ['string', string]
type $character = ['char', string]
type $block = ['block', $call[]]
type $c = ['c', string]
type $expr = $keyword | $number | $block | $string | $c | $character
type $call = ['call', $keyword, $expr[]]
type $fn = ['function', $keyword, $block]
type $action = $call | $fn

type $all = $number | $keyword | $string | $block | $call | $fn | $c | $character

const template = fs.existsSync('template.c') ? fs.readFileSync('template.c').toString().replace(/\\\\n/g, '\\n') : `
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef unsigned char byte;
typedef long long     i4;
#define null NULL

typedef struct Array {
    i4    size;
    i4    unit;
    byte *array;
} *A;

typedef struct Block {
    i4    size;
    i4    used;
    byte *data;
} Block;

#define A(type)                                                                                    \
    struct {                                                                                       \
        i4    size;                                                                                \
        i4    unit;                                                                                \
        type *array;                                                                               \
    }

typedef struct {
    byte initialized;
    A(Block) * blocks;
} Arena;

void arena_free(Arena *arena) {
    if (arena->initialized) {
        for (i4 i = 0; i < arena->blocks->size; i++) {
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
            fprintf(stderr, "Memory allocation failed in alloc!\\n");
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
            return ptr;
        }
    }

    Block new_block;
    new_block.size = (size > SIZE_UNIT) ? size : SIZE_UNIT;
    new_block.used = size;
    new_block.data
        = mmap(null, new_block.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_block.data == NULL) {
        fprintf(stderr, "Memory allocation failed in alloc!\\n");
        exit(EXIT_FAILURE);
    }

    arena->blocks->size++;
    Block* previous         = arena->blocks->array;
    arena->blocks->array = mmap(
        NULL,
        sizeof(Block) * arena->blocks->size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    if (arena->blocks->array == NULL) {
        fprintf(stderr, "Memory allocation failed in alloc (realloc)!\\n");
        exit(EXIT_FAILURE);
    }
    memcpy(arena->blocks->array, previous, sizeof(Block) * (arena->blocks->size - 1));
    munmap(previous, sizeof(Block) * (arena->blocks->size - 1));

    arena->blocks->array[ arena->blocks->size - 1 ] = new_block;

    return new_block.data;
}

A $single(i4 number, Arena *arena) {
    A arr      = alloc(arena, sizeof(struct Array));
    arr->size  = -1;
    arr->array = (byte *) (i4) number;
    return arr;
}

void $log_array(A arr) {
    if (arr->size == -1) {
        printf("[ %lli ]\\n", (i4) arr->array);
    } else {
        printf("[ ");
        for (i4 i = 0; i < arr->size; i++) {
            if (i == arr->size - 1) printf("%d ", arr->array[ i ]);
            else
                printf("%d, ", arr->array[ i ]);
        }
        printf("]\\n");
    }
}

A $multi(i4 size, Arena *arena) {
    A arr      = alloc(arena, sizeof(struct Array));
    arr->size  = size + 1;
    arr->array = alloc(arena, size + 1);
    memset(arr->array, 0, size + 1);
    return arr;
}

A $len(A arr, Arena *arena) {
    if (arr->size == -1) {
        return $single(1, arena);
    } else {
        return $single(arr->size, arena);
    }
}

i4 $value(A item) {
    i4 size = item->size;
    if (size == -1) { return (i4) item->array; }
    if (size > 0) { return item->array[ 0 ]; }

    perror("value Empty array");
    exit(EXIT_FAILURE);
}

byte $is_true(A arr, Arena *arena) {
    i4 size = arr->size;
    if (size == -1) { return (byte) (long) arr->array != 0; }
    if (size == 0) { return 0; }
    for (i4 i = 0; i < size; i++) {
        if (arr->array[ i ] != 0) { return 1; }
    }
    return 0;
}

A $putchar(A value, Arena *arena) {
    i4 size = value->size;
    if (size == -1) {
        putchar((byte) (long) value->array);
    } else if (size > 0) {
        putchar(value->array[ 0 ]);
    } else {
        perror("putchar Empty array");
        exit(EXIT_FAILURE);
    }

    return value;
}

A $get_index(A arr, i4 number, Arena *arena) {
    i4 size = arr->size;
    if (size == -1) {
        if (number == 0) { return $single((byte) (long) arr->array, arena); }

        perror("get_index Invalid index (single-element)");
        exit(EXIT_FAILURE);
    }

    if (number < 0 || number >= size) {
        perror("get_index Out of bounds");
        exit(EXIT_FAILURE);
    }

    return $single(arr->array[ number ], arena);
}

A $get_array(A arr, i4 number, Arena *arena) {
    i4 size = arr->size;
    if (size == -1) { return $single((byte) (long) arr->array, arena); }
    if (arr->array == 0 || size == 0) {
        perror("pop_array Empty array");
        exit(EXIT_FAILURE);
    }

    if (number < 0 || number >= size) {
        perror("pop_array Out of bounds");
        exit(EXIT_FAILURE);
    }

    return $single(arr->array[ number ], arena);
}

A $load_array(const char *str, Arena *arena) {
    A            arr   = $multi(strlen(str), arena);
    struct Array stack = *arr;
    memcpy(stack.array, str, stack.size);
    return arr;
}

#define p_cat(a, b) a##b
#define cat(a, b)   p_cat(a, b)

#define std(name, ...) A cat(fn_, name)(__VA_ARGS__, Arena * arena)

std(dump, A target) {
    $log_array(target);
    return target;
}

std(clone, A target) {
    i4 size = target->size;
    if (size == -1) return $single((i4) target->array, arena);

    A clone = $multi(size, arena);
    memcpy(clone->array, target->array, size);
    return clone;
}

std(push, A _arr, A item) {
    A  arr;
    i4 size = _arr->size;

    if (size == -1) {
        arr             = $multi(2, arena);
        arr->array      = alloc(arena, 2);
        arr->array[ 0 ] = size;
        arr->array[ 1 ] = $value(item);

        return arr;
    } else {
        arr = $multi(size + 1, arena);
        memcpy(arr->array, _arr->array, size);
        arr->array[ size ] = $value(item);
    }

    return arr;
}

std(len, A list) { return $len(list, arena); }
std(get, A arr, A i) { return $get_array(arr, $value(i), arena); }

std(putchar, A val) {
    putchar((byte) $value(val));
    return val;
}

std(add, A left, A right) { return $single($value(left) + $value(right), arena); }
std(sub, A left, A right) { return $single($value(left) - $value(right), arena); }
std(mul, A left, A right) { return $single($value(left) * $value(right), arena); }
std(div, A left, A right) { return $single($value(left) / $value(right), arena); }
std(mod, A left, A right) { return $single($value(left) % $value(right), arena); }
std(and, A left, A right) { return $single($value(left) && $value(right), arena); }
std(or, A left, A right) { return $single($value(left) || $value(right), arena); }
std(not, A target) { return $single($value(target) == 0, arena); }

std(puts, A str) {
    if (str->size == -1) {
        printf("%c", (char) (i4) str->array);
    } else {
        for (i4 i = 0; i < str->size; i++) { printf("%c", str->array[ i ]); }
    }
    return str;
}

std(log, A num) {
    printf("%lli", $value(num));

    return num;
}



`

let glob_id = 0
let string_literal_id = 0
let number_literal_id = 0
let string_literals: { [key: string]: number } = {}
let number_literals: { [key: number]: number } = {}

function emit_c(input: $all) {
    if (input[0] === 'function') {
        return `
A fn_${input[1][1]}(${input[2][1].filter(e => e[1][1] === 'arg').map(e => `A sym_${e[2][0][1]}, `).join('')}Arena* top_scope) {
    Arena cur_scope = {0};

    A output = fn_clone(${emit_c(input[2])}, top_scope);
    arena_free(&cur_scope);
    return output;
}`.trim()
    } else if (input[0] === 'keyword') {
        return `sym_${input[1]}`
    } else if (input[0] === 'block') {
        if (input[1].length === 1) return emit_c(input[1][0])
        return `({ ${input[1].map(e => emit_c(e)).join('; ')}; })`
    } else if (input[0] === 'call') {
        if (input[1][1] === 'while') {
            const id = glob_id++
            return `({ A while_out_${id} = $single(0, &cur_scope); while($is_true(${emit_c(input[2][0])}, &cur_scope)) { while_out_${id} = ${input[2].slice(1).map(e => emit_c(e)).join('; ')}; } while_out_${id}; })`
        } else if (input[1][1] === 'arg') {
            if (input[2][0][0] !== 'keyword') {
                console.error('Arg command must be a keyword')
                process.exit(1)
            }
            return ``
        } else if (input[1][1] === 'list') {
            return `A sym_${input[2][0][1]} = $multi(0, &cur_scope);`
        } else if (input[1][1] === 'num') {
            return `A sym_${input[2][0][1]} = ${emit_c(input[2][1])};`
        } else if (input[1][1] === 'set') {
            return `sym_${input[2][0][1]} = ${emit_c(input[2][1])};`
        } else if (input[1][1] === 'if') {
            return `({ if($is_true(${emit_c(input[2][0])}, &cur_scope)) { ${input[2].slice(1).map(e => emit_c(e)).join('; ')}; } else { $single(0, &cur_scope); } })`
        } else if (input[1][1] === 'unless') {
            return `({ if(!$is_true(${emit_c(input[2][0])}, &cur_scope)) { ${input[2].slice(1).map(e => emit_c(e)).join('; ')}; } else { $single(0, &cur_scope); } })`
        }

        return `fn_${input[1][1]}(${input[2].map(e => `${emit_c(e)}, `).join('')}&cur_scope)`
    } else if (input[0] === 'number') {
        if (number_literals[input[1].toString()] === undefined) {
            number_literals[input[1].toString()] = number_literal_id
            return `NUM__LITERAL_${number_literal_id++}`
        } else {
            return `NUM__LITERAL_${number_literals[input[1].toString()]}`
        }
    } else if (input[0] === 'string') {
        if (string_literals[input[1]] === undefined) {
            string_literals[input[1]] = string_literal_id
            return `STR__LITERAL_${string_literal_id++}`
        } else {
            return `STR__LITERAL_${string_literals[input[1]]}`
        }
    } else if (input[0] === 'c') {
        return input[1]
    } else if (input[0] === 'char') {
        return `$single(${input[1].charCodeAt(0)}, &cur_scope)`
    }

    throw `Invalid type! Object: ${JSON.stringify(input, null, 4)}`
}

if (process.argv.length < 3) {
    console.error('No file specified')
    process.exit(1)
}

const input = fs.readFileSync(process.argv[2]).toString()

const parsed: $action[] = alp.parse(input)
const list_functions = parsed.filter(e => e[0] === 'function').map(e => emit_c(e)).join('\n')
const list_code = parsed.filter(e => e[0] === 'call').map(e => `${emit_c(e)};`).join('\n')
const literal_decl = Object.keys(string_literals).map(e => [e, string_literals[e]]).map(e => `A STR__LITERAL_${e[1]};`).join('\n')
const literal_code = Object.keys(string_literals).map(e => [e, string_literals[e]]).map(e => `STR__LITERAL_${e[1]} = $load_array("${e[0]}", &cur_scope);`).join('\n')
const num_literal_decl = Object.keys(number_literals).map(e => [e, number_literals[e]]).map(e => `A NUM__LITERAL_${e[1]};`).join('\n')
const num_literal_code = Object.keys(number_literals).map(e => [e, number_literals[e]]).map(e => `NUM__LITERAL_${e[1]} = $single(${e[0]}, &cur_scope);`).join('\n')
const main_function = `
int main() {
    Arena cur_scope = {0};

    ${literal_code}
    ${num_literal_code}
    ${list_code}

    arena_free(&cur_scope);

    return 0;
}
`

const code = template + '\n\n' + num_literal_decl + `\n\n` + literal_decl + '\n\n' + list_functions + '\n\n' + main_function

const out_file = process.argv[3] || 'output.a'

const DEBUG = true

const compiler_process = DEBUG ? 
    spawn('clang', ['-o', out_file, '-x', 'c', '-O0', '-pg', '-']) :
    spawn('clang', ['-o', out_file, '-x', 'c', '-O3', '-'])

compiler_process.stdin.write(code)
compiler_process.stdin.end()
fs.writeFileSync('output.c', code)

compiler_process.stdout.on('data', (data) => process.stdout.write(data))
compiler_process.stderr.on('data', (data) => {
    process.stderr.write(data)
})

compiler_process.on('exit', (e) => {
    if (e !== 0) {
        console.error(`Compilation failed with exit code ${e}`)
        process.exit(e)
    }
})