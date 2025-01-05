#pragma once

#include "template.c"
#include "x86_64_linux.c"
// #include "../crew/tourist.c"

void populate(scribe writer, IR ir, ctx context) {
    ground();
    var env = writer.create_env(ir, scratch);

    for (i64 i = ir.segments->size; i >= 0; i--) {
        var segment = ir.segments->array[ i ];

        if (segment.body == null) continue;
        if (segment.body->size == 0) continue;

        writer.block(i, ir, env, scratch);

        for (i64 j = 0; j < segment.body->size; j++) {
            var instruction = segment.body->array[ j ];

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
                    writer.plain_addr(name, env, scratch);
                } else {
                    writer.deref_addr(name, env, scratch);
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

                writer.set(name, env, scratch);
            }

            // print_inst(instruction, ir, context);
            writer.cycle(env, scratch);
        }
    }

    writer.finish(env, scratch);
    release();
}
