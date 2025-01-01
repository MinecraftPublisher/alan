#pragma once

#include <stdint.h>
#include "macros.h"

//

typedef unsigned char byte;
typedef int32_t       i32;
typedef int64_t       i64;

typedef struct Array {
    i32   size;
    i32   unit;
    byte *array;
} *A;

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

typedef A(symbol) * symbolic;

struct standard_entry {
    char *name;
    i32   args;
    char  types[ 256 ];
    char  ret;
};

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

struct compiler_data {
    i32 data_size;

    A(i64) * reserves;
};

typedef struct {
    A(IR_FUNCTION) * segments;
    A(IR_LITERAL) * literals;

    i32 inst_count;
    i32 main_segment;
    i32 global_name_scope_id;

    struct compiler_data *compiler_data;
} IR;

typedef A(byte) * bytecode;

struct Pointer {
    i64 value;
    i64 put_in;
};
typedef A(struct Pointer) * indexes;

typedef struct {
    unsigned char e_ident[ EI_NIDENT ]; // ELF identification
    uint16_t      e_type;               // Object file type
    uint16_t      e_machine;            // Machine type
    uint32_t      e_version;            // Object file version
    uint64_t      e_entry;              // Entry point address
    uint64_t      e_phoff;              // Program header offset
    uint64_t      e_shoff;              // Section header offset
    uint32_t      e_flags;              // Processor-specific flags
    uint16_t      e_ehsize;             // ELF header size
    uint16_t      e_phentsize;          // Size of program header entry
    uint16_t      e_phnum;              // Number of program header entries
    uint16_t      e_shentsize;          // Size of section header entry
    uint16_t      e_shnum;              // Number of section header entries
    uint16_t      e_shstrndx;           // Section header string table index
} Elf64_Ehdr;

void                     __dummy() {}
typedef typeof(__dummy) *mc;