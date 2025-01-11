#pragma once

#include "../types.h"
#include "../macros.h"

#define scribe_sub_section(name, ...) fn(void, cat(__template_scribe_, name) __VA_OPT__(,) __VA_ARGS__, void* environment)

/*
 * Scribe functionality checklist!
 * [ ] useless
 * [ ] nop
 * [ ] pop_to_tmp
 * [ ] push_from_tmp
 * [ ] call
 * [ ] ret
 * [ ] dryback
 * [ ] jmp0
 * [ ] jmpn0
 * [ ] const_to_tmp
 * [ ] addr_deref_to_tmp
 * [ ] addr_no_deref_to_tmp
 * [ ] addr_set_addr_to_tmp
 * [ ] create_block
 * [ ] run_tests
 * [ ] create_env
 * [ ] output // TODO
 */

scribe_sub_section(useless);
scribe_sub_section(nop);
scribe_sub_section(pop_to_tmp);
scribe_sub_section(push_from_tmp);
scribe_sub_section(call, i64 index, IR ref);
scribe_sub_section(ret);
scribe_sub_section(dryback);
scribe_sub_section(jmp0, i64 where);
scribe_sub_section(jmpn0, i64 where);
scribe_sub_section(const_to_tmp, i64 value);
scribe_sub_section(addr_deref_to_tmp, i64 value, i64 sub_top_format);
scribe_sub_section(addr_no_deref_to_tmp, i64 value, i64 sub_top_format);
scribe_sub_section(addr_set_addr_to_tmp, i64 value, i64 sub_top_format);
scribe_sub_section(create_block, i64 name, IR ir);
scribe_sub_section(run_tests, IR ir);
scribe_sub_section(cycle);
scribe_sub_section(finish);
fn(void*, __template_scribe_init_env, IR ir);

typedef struct {
    typeof(__template_scribe_useless)* useless;
    typeof(__template_scribe_nop)* nop;
    typeof(__template_scribe_pop_to_tmp)* pop_tmp;
    typeof(__template_scribe_push_from_tmp)* push_tmp;
    typeof(__template_scribe_call)* call;
    typeof(__template_scribe_ret)* ret_tmp;
    typeof(__template_scribe_dryback)* dryback;
    typeof(__template_scribe_jmp0)* jmp0;
    typeof(__template_scribe_jmpn0)* jmpn0;
    typeof(__template_scribe_const_to_tmp)* constant;
    typeof(__template_scribe_addr_deref_to_tmp)* deref_addr;
    typeof(__template_scribe_addr_no_deref_to_tmp)* plain_addr;
    typeof(__template_scribe_addr_set_addr_to_tmp)* set;
    typeof(__template_scribe_create_block)* block;
    typeof(__template_scribe_run_tests)* tests;
    typeof(__template_scribe_init_env)* create_env;
    typeof(__template_scribe_finish)* finish;
    typeof(__template_scribe_cycle)* cycle;
} scribe;

// #define scribe_debug

#ifdef scribe_debug
#define sdebug(name) printf("SCRIBE " #name "\n")
#else
#define sdebug(name)
#endif