#include "../types.h"

fn(void, __template_scribe_useless, bytecode target) {}
fn(void, __template_scribe_nop, bytecode target) {}
fn(void, __template_scribe_pop_to_tmp, bytecode target) {}
fn(void, __template_scribe_push_from_tmp, bytecode target) {}
fn(void, __template_scribe_call, i64 index, IR ref, bytecode target) {}
fn(void, __template_scribe_ret, bytecode target) {}
fn(void, __template_scribe_jmp0, i64 where, IR ref, bytecode target) {}
fn(void, __template_scribe_jmpn0, i64 where, IR ref, bytecode target) {}
fn(void, __template_scribe_const_to_tmp, i64 value, bytecode target) {}
fn(void, __template_scribe_addr_deref_to_tmp, i64 value, bytecode target) {}
fn(void, __template_scribe_addr_no_deref_to_tmp, i64 value, bytecode target) {}
fn(void, __template_scribe_addr_set_addr_to_tmp, i64 value, bytecode target) {}

typedef struct {
    typeof(__template_scribe_useless)* useless;
    typeof(__template_scribe_nop)* nop;
    typeof(__template_scribe_pop_to_tmp)* pop;
    typeof(__template_scribe_push_from_tmp)* push;
    typeof(__template_scribe_call)* call;
    typeof(__template_scribe_ret)* ret;
    typeof(__template_scribe_jmp0)* jmp0;
    typeof(__template_scribe_jmpn0)* jmpn0;
    typeof(__template_scribe_const_to_tmp)* constant;
    typeof(__template_scribe_addr_deref_to_tmp)* deref_addr;
    typeof(__template_scribe_addr_no_deref_to_tmp)* plain_addr;
    typeof(__template_scribe_addr_set_addr_to_tmp)* set;
} scribe;