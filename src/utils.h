#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//

#include "types.h"
#include "allocator.h"
#include "globals.h"

#include <execinfo.h>
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

string str(char *in, i32 len, Arena *arena) {
    var space = (char *) alloc(arena, len);

    for (i32 i = 0; i < len; i++) space[ i ] = in[ i ];
    space[ len ] = 0;

    return (string) { .unit = 1, .size = len, .array = space };
}

#define __new_array(count, unit, mem) ____new_array(count, unit, (char*)__FUNCTION__, __FILE__, __LINE__, mem)
fn(Array, ____new_array, i64 count, i32 unit, char* function, char* file, int line) {
    adeb(printf(blue("New array ( count %i unit %i ) function %s file %s line %i") "\n", count, unit, function, file, line));
    Array arr      = alloc(mem, sizeof(struct Array));
    arr->size  = count;
    arr->unit  = unit;
    arr->array = alloc(mem, sizeof(byte) * (count + 1) * unit);

    return arr;
}

#define extend(arr, count, mem) __extend(arr, count, mem, (char*)__FUNCTION__, __FILE__, __LINE__)

void __extend(Array array, i64 count, Arena *mem, char* function, char* file, int line) {
    adeb(printf(blue("Extend function %s file %s line %i") "\n", function, file, line));
    var old      = array->array;
    array->array = alloc(mem, sizeof(byte) * (array->size + count + 1) * array->unit);
    memcpy(array->array, old, array->size * array->unit);
    array->size += count;
    // printf("Extend(0x%p, 0x%p);\n", old, array->array);
}

#define copy(arr, mem) (typeof(arr))__copy(arr, mem, (char*)__FUNCTION__, __FILE__, __LINE__)
void *__copy(void *_array, Arena *mem, char* function, char* file, int line) {
    adeb(printf(blue("Copy function %s file %s line %i") "\n", function, file, line));
    Array array          = _array;
    Array new_array      = alloc(mem, sizeof(struct Array));
    new_array->size  = array->size;
    new_array->unit  = array->unit;
    new_array->array = alloc(mem, (new_array->size + 1) * new_array->unit);
    memcpy(new_array->array, array->array, new_array->size * new_array->unit);

    return new_array;
}

byte starts_with(ctx con, char *predicate) {
    return strcmp(&con->str.array[ con->current ], predicate) == 0;
}

void error(ctx con, char *text) {
    // TODO: Improve error reporting, maybe add stack trace?
    if (con == analyzer_error) printf("Inspector Error: %s\n", text);
    else if (con == ir_error) {
        printf("Tourist Error: %s\n", text);
    } else if (con == scribe_error) {
        printf("Scribe Error: %s\n", text);
    } else {
        for (i32 i = 0; i < con->str.size; i++) { putchar(con->str.array[ i ]); }
        printf("\nLibrarian Error: %s\nIndex: %i\n\n", text, con->current);
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