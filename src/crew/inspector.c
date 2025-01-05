#pragma once

#include <stdlib.h>

//

#include "../macros.h"
#include "../types.h"
#include "../printer.h"
#include "../globals.h"
#include "../utils.h"

fn(exp_type, __validate, i val, symbolic symbols, ctx context, exp_type top_type) {
    if (val == null) return exp_void;

#ifdef enable_debug
    print_i(val, 0, context, 0);

    for (i32 i = 0; i < symbols->size; i++) printf("\nsym %i\n", symbols->array[ i ]);
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
        printf("\n\nReference index: %i\n", val->value.ref);
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
                    "<Overlord> Hey, there are way too many references to the number %i. You "
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

        // for (i32 i = 0; i < val->value.fn.args->size; i++) {
        //     push(clone, val->value.fn.args->array[ i ], mem);
        // }

        __validate(val->value.fn.block, symbols, context, val->value.fn.type, mem);

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
        } else if (val->value.call.name == SET_CALL) {
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
        } else if (val->value.call.name == RET_CALL) {
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
        } else if (val->value.call.name == DRYBACK_CALL) {
            if (val->value.call.args->size == 0) {
                if (top_type == exp_void) return 0;
                else {
                    print_i(val, 0, context, 0);
                    printf("\n");
                    error(
                        analyzer_error,
                        "A dry return command in a non-void function must have a return value!");
                }
            } else if (top_type == exp_void) {
                print_i(val, 0, context, 0);
                printf("\n");
                error(analyzer_error, "A dry return command in a void function takes zero arguments!");
            }

            var value = val->value.call.args->array[ 0 ];
            var type  = __validate(value, symbols, context, top_type, mem);

            if (type != top_type) {
                print_i(val, 0, context, 0);
                printf("\n");
                error(analyzer_error, "Dry return command's argument must match the function's type!");
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
                        "\n\nReferenced call: '%s' on argument %i:\n    ",
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
                        "\n\nReferenced call: '%s' on argument %i:\n    ",
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

        val->value.block.symbols = null;
        val->value.block.symbols = (void*)cloned_syms;

        exp_type output = exp_void;
        for (i32 i = 0; i < val->value.block.items->size; i++) {
            output = __validate(val->value.block.items->array[ i ], cloned_syms, context, top_type, mem);
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