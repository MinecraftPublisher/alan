#pragma once

#include <stdio.h>
#include <stdlib.h>

//

#include "types.h"

void print_i(i val, i32 indent, ctx con, byte print_sym) {
    if (val == null) printf("\r");

    else if (val->type == tref) {
        if (print_sym) printf("<$%i>", val->value.ref);
        else { printf("%s", con->symbols->array[ labs(val->value.ref) ]); }
    }

    else if (val->type == tchr) {
        printf("'%c'", val->value.chr);
    }

    else if (val->type == tstr) {
        if (print_sym) printf("<\"%i>", val->value.str_id);
        else { printf("\"%s\"", con->literals->array[ val->value.str_id ].value.str.array); }
    }

    else if (val->type == tnum) {
        if (print_sym) printf("<#%i>", val->value.str_id);
        else { printf("%i", con->literals->array[ val->value.str_id ].value.num); }
    }

    else if (val->type == tfn) {
        if (print_sym) printf("\nfn[%i] <$%i>(", val->value.fn.type, val->value.fn.name);
        else {
            printf(
                "fn[%s] %s (",
                val->value.fn.type == exp_num    ? "num"
                : val->value.fn.type == exp_list ? "list"
                                                 : "void",
                con->symbols->array[ val->value.fn.name ]);
        }
        for (i32 i = 0; i < val->value.fn.args->size; i++) {
            if (print_sym) printf("<$%i>", val->value.fn.args->array[ i ]);
            else { printf("%s", con->symbols->array[ labs(val->value.fn.args->array[ i ]) ]); }
            if (i + 1 < val->value.fn.args->size) printf(", ");
        }
        printf(") ");

        print_i(val->value.fn.block, indent, con, print_sym);
        printf(";\n");
    }

    else if (val->type == tcall) {
        if (print_sym) printf("<$%i>", val->value.call.name);
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