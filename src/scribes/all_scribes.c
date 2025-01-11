#pragma once

#include "template.c"
#include "x86_64_linux.c"
// #include "../crew/tourist.c"

void cycle(scribe writer, IR_FUNCTION segment, i64 i, IR ir, void* env, Arena* scratch) {
    writer.block(i, ir, env, scratch);

    for (i64 j = 0; j < segment.body->list->size; j++) {
        var instruction = segment.body->list->array[ j ];

        if (instruction.op == iruseless) {
            writer.useless(env, scratch);
        } else if (instruction.op == irpop) {
            writer.pop_tmp(env, scratch);
        } else if (instruction.op == irpush) {
            // if (segment.body->array[ j + 1 ].op == irpop) {
            // j++;
            // continue;
            // }
            writer.push_tmp(env, scratch);
        } else if (instruction.op == ircall) {
            writer.call(instruction.data.icall.ref, ir, env, scratch);
        } else if (instruction.op == irret) {
            writer.ret_tmp(env, scratch);
        } else if (instruction.op == irdryback) {
            writer.dryback(env, scratch);
        } else if (instruction.op == irjmp0) {
            writer.jmp0(instruction.data.ijmp0.ref, env, scratch);
        } else if (instruction.op == irjmpn0) {
            writer.jmpn0(instruction.data.ijmpn0.ref, env, scratch);
        } else if (instruction.op == irconst) {
            writer.constant(instruction.data.iconstant.value, env, scratch);
        } else if (instruction.op == iraddr) {
            var noderef = instruction.data.iaddr.value < 0;

            var my_name = llabs(instruction.data.iaddr.value);
            var name    = -1;

            for (i64 i = 0; i < ir.compiler_data.reserves->size; i++) {
                if (ir.compiler_data.reserves->array[ i ] == my_name) {
                    name = i;
                    break;
                }
            }

            if (name == -1) { error(scribe_error, "Could not find variable name!"); }

            if (noderef) {
                writer.plain_addr(name, my_name, env, scratch);
            } else {
                writer.deref_addr(name, my_name, env, scratch);
            }
        } else if (instruction.op == irset) {
            var my_name = llabs(instruction.data.iset.dest);
            var name    = -1;

            for (i64 i = 0; i < ir.compiler_data.reserves->size; i++) {
                if (ir.compiler_data.reserves->array[ i ] == my_name) {
                    name = i;
                    break;
                }
            }

            if (name == -1) { error(scribe_error, "Could not find variable name!"); }

            writer.set(name, my_name, env, scratch);
        }

        // print_inst(instruction, ir, context);
        writer.cycle(env, scratch);
    }
}

void populate(scribe writer, IR ir, ctx context) {
    ground();
    var env = writer.create_env(ir, scratch);

    i64 main_segment = -1;

    for (i64 i = 0; i < ir.segments->size; i++) {
        var segment = ir.segments->array[ i ];

        if (segment.body == null) continue;
        if (segment.body->list->size == 0) continue;
        if (segment.name == 1) {
            main_segment = i;
            continue;
        }

        cycle(writer, segment, i, ir, env, scratch);
    }

    if(main_segment != -1) cycle(writer, ir.segments->array[ main_segment ], main_segment, ir, env, scratch);

    writer.finish(env, scratch);
    release();
}
