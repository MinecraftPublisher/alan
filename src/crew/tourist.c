#pragma once

#include <stdio.h>

//

#include "../globals.h"
#include "../macros.h"
#include "../printer.h"
#include "../types.h"
#include "../utils.h"

void print_inst(IR_INST inst, IR scope, ctx context) {
    ground();

    switch (inst.op) {
        case irpop: printf("POP\n"); break;
        case irpush: printf("PUSH\n"); break;
        case ircall:;
            var my_name = scope.segments->array[ inst.data.icall.ref ].name;
            printf(
                red("CALL") yellow(" __0x%X ") POS_ALIGN "; %s\n",
                my_name < 0 ? -my_name : inst.data.icall.ref,
                my_name < 0 ? "block" : context->symbols->array[ my_name ]);
            break;
        case irret: printf("RET\n"); break;
        case irdryback: printf("DRYBACK\n"); break;
        case irjmp0: printf(red("JMP0 ") yellow("__0x%X\n"), inst.data.ijmp0.ref); break;
        case irjmpn0: printf(red("JMPN0") yellow(" __0x%X\n"), inst.data.ijmpn0.ref); break;
        case irconst: printf(magenta("CONST ") green("%i\n"), inst.data.iconstant.value); break;
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

            // REFACTOR: Make this readable!
            printf(
                bright_blue("ADDR") " "
                                    "%s" yellow("0x%llX_%llX") "%s" POS_ALIGN "; %s%s%s\n",
                noderef ? "" : "[",
                is_str ? 0 : topindex,
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
                green("SET") " " yellow("0x%llX_%llx") POS_ALIGN "; %s\n",
                topindex,
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

    i32 skips         = 0;
    i64 skip_segments = -stdlib_size;

    printf("== Intermediate Representation ==\n\n");
    for (i32 i = scope.segments->size; i >= 0; i--) {
        var cur = scope.segments->array[ i ];

        if (*(i32 *) &cur == 0) continue;
        skip_segments++;
        if (cur.block->list->size == 0) continue;
        skip_segments--;

        if (cur.name == 1) printf(bold_blue("main") ":\n");
        else { printf(bold_blue("0x%X") ":\n", i); }
        for (i32 j = 0; j < cur.block->list->size; j++) {
            var inst = cur.block->list->array[ j ];
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
        "=== Tourist's Take ===\n - %i instruction%s\n - %li literal%s\n - %li segment%s (%li "
        "total, %li builtin)\n\n",
        scope.inst_count - skips,
        scope.inst_count - skips == 1 ? "" : "s",
        scope.literals->size,
        scope.literals->size == 1 ? "" : "s",
        scope.segments->size - skip_segments - stdlib_size * 2,
        (scope.segments->size - skip_segments - stdlib_size * 2) == 1 ? "" : "s",
        scope.segments->size - skip_segments - stdlib_size,
        stdlib_size);

    /*printf("== Literals ==\n");
    for (i32 i = 0; i < scope.literals->size; i++) {
        var item = scope.literals->array[ i ];
        scope.compiler_data.data_size += item.type == ir_lnum ? 0 : item.value.string.size;
        printf(
            " - Index(%i) Type(%s) | Value(%s%s%s)\n",
            i,
            item.type == ir_lnum ? "number" : "string",
            item.type == ir_lstr ? "\"" : "",
            item.type == ir_lstr ? item.value.string.array : ltoa(item.value.number, scratch),
            item.type == ir_lstr ? "\"" : "");
    }*/

    release();
}

fn(void, represent, i64 parent, i trm, IR *scope, IR_SCOPE names, INST_LIST cur, ctx ctx) {
    if (trm == null) return;

    scope->inst_count++;

    var result   = (IR_INST) { .op = iruseless };
    result.scope = names;

    if (trm->type == tchr) {
        result.op                   = irconst;
        result.data.iconstant.value = trm->value.chr;
    } else if (trm->type == tnum) {
        result.op                   = irconst;
        result.data.iconstant.value = ctx->literals->array[ trm->value.num_id ].value.num;
    } else if (trm->type == tstr) { // what to do here?
        result.op                  = iraddr;
        result.data.iaddr.value    = -trm->value.str_id;
        result.data.iaddr.actuator = 1;
    } else if (trm->type == tref) {
        result.op  = iraddr;
        var target = labs(trm->value.ref);

        for (i32 i = names->size - 1; i >= 0; i--) {
            var scope = names->array[ i ];

            for (i32 j = 0; j < scope->size; j++) {
                var cur = scope->array[ j ];
                if (cur != target) continue;

                result.data.iaddr.value = ((i64) i << 32) + (i64) j;
                // printf("Is(%i) %llX\n", context->literals->array[term->value.ref].type,
                // result.data.iaddr.value);
                result.data.iaddr.value *= trm->value.ref > 0 ? -1 : 1;
                // printf("%s %li\n", context->symbols->array[target], term->value.ref);

                for (i32 i = 0; i < stdlib_size; i++) {
                    if (stdlib[ i ].ref == target) {
                        print_i(trm, 0, ctx, 0);
                        printf("\n\n");
                        error(ir_error, "Restricted reference!");
                    }
                }

                goto END;
            }
        }

        print_i(trm, 0, ctx, 0);
        printf("\n\n");
        error(ir_error, "Invalid or unknown reference!");
    } else if (trm->type == tcall) {
        var name = trm->value.call.name;

        if (name == NUM_TYPE) {
            result.op  = irset;
            var target = trm->value.call.args->array[ 0 ]->value.ref;

            push(names->array[ names->size - 1 ], target, mem);

            var topindex = (i64) names->size - 1;
            var subindex = (i64) names->array[ names->size - 1 ]->size - 1;

            result.data.iset.dest = (topindex << 32) + subindex;

            var resolved = llabs(result.data.iset.dest);
            push(scope->compiler_data.reserves, resolved, mem);

            represent(parent, trm->value.call.args->array[ 1 ], scope, names, cur, ctx, mem);

            cur->stack_size++;

            goto END;
        } else if (name == LIST_TYPE) {
            // Initialize a list
            var target = trm->value.call.args->array[ 0 ]->value.ref;
            push(names->array[ names->size - 1 ], target, mem);

            represent(parent, trm->value.call.args->array[ 0 ], scope, names, cur, ctx, mem);

            var resolved = llabs(cur->list->array[ cur->list->size - 1 ].data.iaddr.value);
            var subindex = (resolved << 32) >> 32;
            var topindex = resolved >> 32;
            push(scope->compiler_data.reserves, resolved, mem);

            result.op = ircall;

            var real_name = scope->segments->array[ name ].name;

            for (i32 i = 0; i < scope->segments->size; i++) {
                if (scope->segments->array[ i ].name != LIST_TYPE) continue;
                real_name = i;
            }

            result.data.icall.ref = real_name;

            cur->stack_size++;

            goto END;
        } else if (name == IF_CALL) {
            var seg_name  = scope->segments->size;
            var me        = (IR_FUNCTION) { .name = -seg_name, .parent = parent, .block = cur };
            var new_scope = (IR_SCOPE_ARRAY) ret(i32, 0);
            if (parser_optimizations <= OPT_BASIC) {
                me.block       = ret(struct INST_LIST);
                me.block->list = (void *) ret(IR_INST, 0);
                push(scope->segments, me, mem);
            }

            var my_scope = (IR_SCOPE) copy(names, mem);
            push(my_scope, new_scope, mem);
            represent(seg_name, trm->value.call.args->array[ 0 ], scope, names, cur, ctx, mem);

            result.op              = irjmpn0;
            result.data.ijmpn0.ref = seg_name;

            represent(
                scope->segments->size - 1,
                trm->value.call.args->array[ 1 ],
                scope,
                my_scope,
                (void *) me.block,
                ctx,
                mem);

            // cur->size--;

            goto END;
        } else if (name == UNLESS_CALL) {
            var seg_name = scope->segments->size;
            var me = (IR_FUNCTION) { .name = -seg_name, .parent = parent, .block = (void *) cur };
            var new_scope = (IR_SCOPE_ARRAY) ret(i32, 0);
            if (parser_optimizations <= OPT_BASIC) {
                me.block       = ret(struct INST_LIST);
                me.block->list = (void *) ret(IR_INST, 0);
                push(scope->segments, me, mem);
            }

            var my_scope = (IR_SCOPE) copy(names, mem);
            push(my_scope, new_scope, mem);
            represent(parent, trm->value.call.args->array[ 0 ], scope, names, cur, ctx, mem);

            result.op             = irjmp0;
            result.data.ijmp0.ref = seg_name;

            represent(
                seg_name,
                trm->value.call.args->array[ 1 ],
                scope,
                my_scope,
                (void *) me.block,
                ctx,
                mem);

            goto END;
        } else if (name == WHILE_CALL) {
            var seg_name = scope->segments->size;
            var me = (IR_FUNCTION) { .name = -seg_name, .parent = parent, .block = (void *) cur };
            var new_scope = (IR_SCOPE_ARRAY) ret(i32, 0);
            if (parser_optimizations <= OPT_BASIC) {
                me.block       = ret(struct INST_LIST);
                me.block->list = (void *) ret(IR_INST, 0);
                push(scope->segments, me, mem);
            }

            var my_scope = (IR_SCOPE) copy(names, mem);
            push(my_scope, new_scope, mem);
            represent(parent, trm->value.call.args->array[ 0 ], scope, names, cur, ctx, mem);

            result.op              = irjmpn0;
            result.data.ijmpn0.ref = seg_name;

            represent(
                seg_name,
                trm->value.call.args->array[ 1 ],
                scope,
                my_scope,
                (void *) me.block,
                ctx,
                mem);

            represent(
                seg_name,
                trm->value.call.args->array[ 0 ],
                scope,
                my_scope,
                (void *) me.block,
                ctx,
                mem);

            push(me.block->list, result, mem);
            scope->inst_count++;

            goto END;
        } else if (name == SET_CALL) {
            represent(parent, trm->value.call.args->array[ 1 ], scope, names, cur, ctx, mem);

            var target   = trm->value.call.args->array[ 0 ]->value.ref;
            var resolved = (i64) -1;

            for (i32 i = names->size - 1; i >= 0; i--) {
                var scope = names->array[ i ];

                for (i32 j = 0; j < scope->size; j++) {
                    var cur = scope->array[ j ];
                    if (labs(cur) != labs(target)) continue;

                    resolved = (((i64) i) << 32) + (i64) j;
                    goto SET_FINISH;
                }
            }

            push(names->array[ names->size - 1 ], target, mem);
            resolved = (((i64) names->size - 1) << 32)
                       + (i64) (names->array[ names->size - 1 ]->size - 1);
            push(scope->compiler_data.reserves, resolved, mem);

            cur->stack_size++;

        SET_FINISH:
            result.op             = irset;
            result.data.iset.dest = resolved;

            goto END;
        } else if (name == ARG_TYPE) {
            // FIXME
            var pop_stack = (IR_INST) { .op = irpop, .data.ipop = {} };

            push(cur->list, pop_stack, mem);
            scope->inst_count++;

            var target   = trm->value.call.args->array[ 1 ]->value.ref;
            var resolved = (i64) -1;

            for (i32 i = names->size - 1; i >= 0; i--) {
                var _scope = names->array[ i ];

                for (i32 j = 0; j < _scope->size; j++) {
                    var cur = _scope->array[ j ];
                    if (labs(cur) != labs(target)) continue;

                    resolved = (((i64) i) << 32) + (i64) j;
                    push(scope->compiler_data.reserves, resolved, mem);
                    goto ARG_FINISH;
                }
            }

            // error here
            print_i(trm, 0, ctx, 0);
            printf("\n\n");
            error(ir_error, "Argument reference was not found! Is this a bug?");

        ARG_FINISH:

            cur->stack_size++;

            result.op             = irset;
            result.data.iset.dest = resolved;

            goto END;
        } else if (name == RET_CALL) {
            for (i32 i = 0; i < trm->value.call.args->size; i++) {
                represent(parent, trm->value.call.args->array[ i ], scope, names, cur, ctx, mem);
            }

            result.op = irret;

            goto END;
        } else if (name == DRYBACK_CALL) {
            for (i32 i = 0; i < trm->value.call.args->size; i++) {
                represent(parent, trm->value.call.args->array[ i ], scope, names, cur, ctx, mem);
            }

            result.op = irdryback;

            goto END;
        }

        for (i32 i = trm->value.call.args->size - 1; i >= 0; i--) {
            represent(parent, trm->value.call.args->array[ i ], scope, names, cur, ctx, mem);
            push(cur->list, push_stack, mem);
            scope->inst_count++;
        }

        result.op = ircall;

        var real_name = scope->segments->array[ name ].name;

        for (i32 i = 0; i < scope->segments->size; i++) {
            if (scope->segments->array[ i ].name != name) continue;
            real_name = i;
        }

        result.data.icall.ref = real_name;
    } else if (trm->type == tcode) {
        if (trm->value.block.items->size >= 16 || parser_optimizations > OPT_BASIC) {
            var seg_name = scope->segments->size;
            var me       = (IR_FUNCTION) { .name   = -seg_name,
                                           .parent = parent,
                                           .block  = ret(struct INST_LIST) };
            push(scope->segments, me, mem);

            var new_scope = (IR_SCOPE_ARRAY) ret(i32, 0);
            var my_scope  = (IR_SCOPE) copy(names, mem);
            push(my_scope, new_scope, mem);

            for (i32 i = 0; i < trm->value.block.items->size; i++) {
                represent(
                    seg_name,
                    trm->value.block.items->array[ i ],
                    scope,
                    my_scope,
                    (INST_LIST) me.block,
                    ctx,
                    mem);
            }

            result.op             = ircall;
            result.data.icall.ref = seg_name;
        } else {
            for (i32 i = 0; i < trm->value.block.items->size; i++) {
                represent(parent, trm->value.block.items->array[ i ], scope, names, cur, ctx, mem);
            }

            // result.op = irnop;
        }
    } else if (trm->type == tfn) {
        var my_segment         = (IR_FUNCTION) { .name   = trm->value.fn.name,
                                                 .parent = parent,
                                                 .block  = ret(struct INST_LIST) };
        my_segment.block->list = (void *) ret(IR_INST, 0);
        push(scope->segments, my_segment, mem);

        var new_scope = (IR_SCOPE_ARRAY) ret(i32, 0);

        for (i32 i = 0; i < trm->value.fn.args->size; i++) {
            push(new_scope, trm->value.fn.args->array[ i ], mem);
        }

        var my_scope = (IR_SCOPE) copy(names, mem);
        push(my_scope, new_scope, mem);

        for (i32 i = 0; i < trm->value.fn.block->value.block.items->size; i++) {
            represent(
                trm->value.fn.name,
                trm->value.fn.block->value.block.items->array[ i ],
                scope,
                my_scope,
                (INST_LIST) my_segment.block,
                ctx,
                mem);
        }

        // TODO! return existence check???
    } else if (trm->type == tembd) {
        print_i(trm, 0, ctx, 0);
        printf("\n\n");
        error(ir_error, "Embedded direct code is not supported!");
    } else {
        print_i(trm, 0, ctx, 0);
        printf("\n\n");
        error(ir_error, "Unknown AST node type!");
    }

END:
    // printf("```\n");

    // print_i(term, 0, context, 0);
    // printf("\n");

    // print_inst(result, context);

    // printf("```\n\n\n");

    result.scope = names;

    push(cur->list, result, mem);
    // if (result.op == irconst) push(cur, push_stack, mem);
}

fn(IR, emit, A(i) * term, ctx context) {
    var output                    = (IR) { .segments      = (void *) ret(IR_FUNCTION, 0),
                                           .literals      = (void *) ret(IR_LITERAL, 0),
                                           .inst_count    = 0,
                                           .compiler_data = { .reserves = (void *) ret(i64, 0), .context = context } };
    output.compiler_data.reserves = (void *) ret(i64, 0);

    var main_segment = (IR_FUNCTION) { .name = 1, .parent = -1, .block = ret(struct INST_LIST) };
    main_segment.block->list = (void *) ret(IR_INST, 0);

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
            var func         = (IR_FUNCTION) { .name   = item.value.fn.name,
                                               .parent = -1,
                                               .block  = ret(struct INST_LIST) };
            func.block->list = (void *) ret(IR_INST, 0);

            push(output.segments, func, mem);
        } else {
            error(ir_error, "Unknown literal type!");
        }
    }

    // var push_stack = (IR_INST) { .op = irpush, .data.ipush = {} };

    var scopes     = ret(IR_SCOPE_ARRAY, 0);
    var main_scope = (IR_SCOPE_ARRAY) ret(i32, 0);

    push(scopes, main_scope, mem);

    // Compute normal terms
    for (i32 i = 0; i < term->size; i++) {
        var item = term->array[ i ];
        if (item == null) continue;

        represent(-1, item, &output, (void *) scopes, (void *) main_segment.block, context, mem);
    }

    return output;
}