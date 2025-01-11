#pragma once

//

#include "../globals.h"
#include "../macros.h"
#include "../types.h"
#include "../utils.h"

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

void parse_comment(ctx con) {
    if (cur() == '#' || (cur() == '/' && cur(1) == '/')) { while (skip() != '\n' && !eof()); }
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
    val->value.block.items = (void *) ret(i, 0);

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
        if (ex != null) push(val->value.block.items, ex, mem);
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

    if (val->value.block.items->size == 0) error(con, "Empty blocks are not supported!");

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
        printf("%i\n", type);
        error(con, "Expected function type when parsing function...");
    }
    parse_null(con);

    var prev_size = con->symbols->size;
    var fn_name = parse_ref_str(con, mem);
    if(con->symbols->size == prev_size) {
        error(con, "Duplicate function name!");
    }
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

    for (i32 i = 0; i < code->value.block.items->size; i++) {
        var current = code->value.block.items->array[ i ];
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
            // code->value.block.array[ i ] = null;
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
