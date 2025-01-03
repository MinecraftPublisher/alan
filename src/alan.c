// #define arena_debug
// #define enable_debug

#include <stdio.h>
#include <string.h>

//

#include "types.h"

// ======== Librarian (Parser) ========
#include "crew/librarian.c"

// ======== Inspector (Static analyzer) ========
#include "crew/inspector.c"

// ======== Tourist (Intermediate Representation Generator) ========
#include "crew/tourist.c"

// ======== Scribes (codegen backends) ========
#include "scribes/all_scribes.c"

// ======== Setup functions ========

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
        if (!strcmp(stdlib[ i ].name, "set")) SET_CALL = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "ret")) RET_CALL = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "if")) IF_CALL = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "unless")) UNLESS_CALL = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "while")) WHILE_CALL = con->symbols->size - 1;
        if (!strcmp(stdlib[ i ].name, "dryback")) DRYBACK_CALL = con->symbols->size - 1;

        stdlib[ i ].ref = con->symbols->size - 1;
    }

    return con;
}

A(i) * main_parse(ctx con, Arena *mem) {
    var terms = ret (i, 0);

    parse_null(con);
    while (!eof()) {
        var result = parse_action(con, mem);
        if (result == null) {
            parse_null(con);
            if (!eof()) { error(con, "Invalid syntax!"); }
        }

        push(terms, result, mem);
        if (skip() != ';') error(con, "Expected semicolon...");
        parse_null(con);
    }

    return (void *) terms;
}

// ======== Main code ========

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

    // Print AST structure
    for (i32 i = 0; i < terms->size; i++) {
        print_i(terms->array[ i ], 0, con, 0);
        printf("\n");
    }

    // Generate intermediate representation
    var ir = emit((void *) terms, con, scratch);

    printf("\n\n");
    tourist(ir, con, scratch);
    printf("\n");

    // x86 test

    var current    = get_scribe_x86_64_linux();
    // var scribe_env = current.create_env(ir, scratch);

    // current.tests(ir, scribe_env, scratch);

    populate(current, ir);

    // release();
}