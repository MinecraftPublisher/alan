#pragma once

#include "types.h"

char nullchar = '\0';

const var scribe_error   = (void *) -3;
const var ir_error       = (void *) -2;
const var analyzer_error = (void *) -1;

// zero shows maximum optimization level. sacrifice compile-time for run-time.
const enum {
    OPT_AGGRESSIVE = 0,
    OPT_BASIC      = 1,
    OPT_MEDIUM     = 2,
    OPT_NONE       = 10
} parser_optimizations
    = OPT_AGGRESSIVE;

byte suggestions = 1;

// 0 shows any type (list or int)
// 1 shows list type
// -1 shows number type
// 2 shows block type
// 3 shows void type

// struct standard_entry stdlib[] = {
//     // Internals
//     { .name = "ret", .args = 1, .types = { 0 }, .ret = 3 },       //
//     { .name = "arg", .args = 2, .types = { 0, 0 }, .ret = 3 },    //
//     { .name = "if", .args = 2, .types = { 0, 2 }, .ret = 3 },     //
//     { .name = "unless", .args = 2, .types = { 0, 2 }, .ret = 3 }, //
//     { .name = "while", .args = 2, .types = { 0, 2 }, .ret = 3 },  //
//     // I/O functions
//     { .name = "dump", .args = 1, .types = { 0 }, .ret = 3 },   //
//     { .name = "puts", .args = 1, .types = { 1 }, .ret = 1 },   //
//     { .name = "putc", .args = 1, .types = { -1 }, .ret = -1 }, //
//     { .name = "log", .args = 1, .types = { 0 }, .ret = 3 },    //
//     { .name = "print", .args = 1, .types = { 1 }, .ret = 3 },  //
//     // Arithmetic and logic
//     // TODO: Add higher number arithmetics
//     { .name = "add", .args = 2, .types = { -1, -1 }, .ret = -1 },
//     { .name = "sub", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
//     { .name = "mul", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
//     { .name = "div", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
//     { .name = "mod", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
//     { .name = "and", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
//     { .name = "or", .args = 2, .types = { -1, -1 }, .ret = -1 },     //
//     { .name = "not", .args = 1, .types = { -1 }, .ret = -1 },        //
//     { .name = "dec", .args = 1, .types = { -1 }, .ret = -1 },        //
//     { .name = "inc", .args = 1, .types = { -1 }, .ret = -1 },        //
//     { .name = "cmp_gt", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
//     { .name = "cmp_lt", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
//     { .name = "cmp_eq", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
//     { .name = "cmp_ne", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
//     { .name = "cmp_ge", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
//     { .name = "cmp_le", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
//     // Array manipulation
//     { .name = "get", .args = 2, .types = { 1, -1 }, .ret = -1 }, // get(list, index): num
//     { .name = "push", .args = 2, .types = { 1, -1 }, .ret = 1 }, // push(list, value): list
//     { .name = "len", .args = 1, .types = { 1 }, .ret = -1 },     //
//     { .name = "num", .args = 2, .types = { 0 }, .ret = -1 },     //
//     { .name = "list", .args = 1, .types = { 0 }, .ret = 1 },     //
//     { .name = "set", .args = 2, .types = { 0, 0 }, .ret = 3 },   //
//     { .name = "len", .args = 1, .types = { 1 }, .ret = -1 },     //
//     // Threading
//     { .name = "spawn", .args = 1, .types = { 2 }, .ret = -1 },   // fork(): id
//     { .name = "send", .args = 2, .types = { -1, 1 }, .ret = 3 }, // send(thread, value): void
//     { .name = "fetch", .args = 0, .types = {}, .ret = 1 }        //
// };
// const var stdlib_size = (sizeof(stdlib) / sizeof(stdlib[ 0 ]));

// DRY STDLIB
// USED FOR LANGUAGE DEVELOPMENT
struct standard_entry stdlib[] = {
    // Internals
    { .name = "mmap", .args = 1, .types = { -1 }, .ret = -1 },    //
    { .name = "munmap", .args = 1, .types = { -1 }, .ret = 0 },   //
    { .name = "deref", .args = 1, .types = { -1 }, .ret = 0 },    // Dereferences a pointer, and places it into TMP.
    { .name = "dryback", .args = 1, .types = { 0 }, .ret = 3 },   //
    { .name = "arg", .args = 2, .types = { 0, 0 }, .ret = 3 },    //
    { .name = "if", .args = 2, .types = { 0, 2 }, .ret = 3 },     //
    { .name = "unless", .args = 2, .types = { 0, 2 }, .ret = 3 }, //
    { .name = "while", .args = 2, .types = { 0, 2 }, .ret = 3 },  //
    // Arithmetic and logic
    // TODO: Add higher number arithmetics
    { .name = "add", .args = 2, .types = { -1, -1 }, .ret = -1 },
    { .name = "sub", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "mul", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "div", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "and", .args = 2, .types = { -1, -1 }, .ret = -1 },    //
    { .name = "or", .args = 2, .types = { -1, -1 }, .ret = -1 },     //
    { .name = "not", .args = 1, .types = { -1 }, .ret = -1 },        //
    { .name = "inc", .args = 1, .types = { -1 }, .ret = -1 },        //
    { .name = "dec", .args = 1, .types = { -1 }, .ret = -1 },        //
    { .name = "cmp_gt", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_lt", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_eq", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_ne", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_ge", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    { .name = "cmp_le", .args = 2, .types = { -1, -1 }, .ret = -1 }, //
    // Array manipulation
    { .name = "get", .args = 2, .types = { 1, -1 }, .ret = -1 }, // get(list, index): num
    { .name = "len", .args = 1, .types = { 1 }, .ret = -1 },     //
    { .name = "num", .args = 2, .types = { 0 }, .ret = -1 },     //
    { .name = "list", .args = 1, .types = { 0 }, .ret = 1 },     //
    { .name = "set", .args = 2, .types = { 0, 0 }, .ret = 3 },   //
};
const var stdlib_size = (sizeof(stdlib) / sizeof(stdlib[ 0 ]));

const IR_INST push_stack = { .op = irpush, .data = {} };
const IR_INST nop        = { .op = irnop, .data.nop = {} };

byte NUM_TYPE;
byte LIST_TYPE;
byte ARG_TYPE;
byte SET_CALL;
byte RET_CALL;
byte IF_CALL;
byte UNLESS_CALL;
byte WHILE_CALL;
byte VOID_TYPE;
byte MMAP_CALL;
byte RAW_SEG_CALL;
byte MMAP_CALL;
byte DRYBACK_CALL;