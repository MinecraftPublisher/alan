#include "../macros.h"
#include "../types.h"
#include "../utils.h"

//

#include "template.c"

//

#include <stdint.h>

typedef A(byte) * bytecode;

struct Pointer {
    i64 value;
    i64 put_in;
};
typedef A(struct Pointer) * indexes;

struct block_loc {
    i64 alan_name;
    i64 actual_address;
};

#define MAIN_STACK_SIZE         ((1 << 13) * sizeof(i64) / 8)
#define RESTORE_MAIN_STACK_SIZE ((1 << 8) * sizeof(i64) / 8)
#define VAR_STACK_SIZE          ((1 << 13) * sizeof(i64) / 8)
#define RESTORE_VAR_STACK_SIZE  ((1 << 8) * sizeof(i64) / 8)

typedef struct x86_64_linux_env {
    A(struct block_loc) * block_locations;
    indexes  replace_pointers;
    bytecode target;
    IR       ir;

    i64 func_start;
    i64 put_main;
    i64 jump_after;

    i64 main_segment; // SET AFTER INIT
    i64 main_index;
    i64 allocator; // SET AFTER INIT

    i64 cur_block_size;
    i64 cur_block_index;
    i64 stack_diff;

    i64 push_ptr;
    i64 pop_ptr;
    i64 push_restore_ptr;
    i64 pop_restore_ptr;

    i64 last_push_index;

    struct {
        i64 *main_stack;
        i64 *restore_main_stack;

        i64 *var_stack;
    } stacks;
} *x86_64_linux_env;

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

/*
 * Scribe functionality checklist! (x86_64_linux)
 * [x] useless
 * [x] nop
 * [x] pop_to_tmp
 * [x] push_from_tmp
 * [ ] call
 * [ ] ret
 * [x] dryback
 * [x] jmp0
 * [x] jmpn0
 * [x] const_to_tmp
 * [x] addr_deref_to_tmp
 * [x] addr_no_deref_to_tmp
 * [x] addr_set_addr_to_tmp
 * [x] create_block
 * [x] run_tests
 * [x] create_env
 */

// [x] create_block();
// [x] pop(mov [cur_stack], %rex);
// [x] push(push_stack %rex);
// [ ] call(complicated...);
// [ ] ret(complicated...);
// [x] jmp0(cmp %rex; jmp0 [addr]);
// [x] jmpn0(reverse of jmp0)
// [x] addrI(mov %rex, addr);
// [x] addrD(mov %rex, [addr]);
// [x] set(mov [addr], %rex);
// [x] const(mov %rex, $value);
// [x] nop(); # Why would I even add this?

fn(void, __x86_64_linux_machine_int3, bytecode target) {
    // return;
    push(target, 0xcd, mem);
    push(target, 0x03, mem);
}

fn(void, __x86_64_linux_machine_emit_qword, i64 value, bytecode target) {
    push(target, (byte) (value & 0xff), mem);
    push(target, (byte) ((value >> 8) & 0xff), mem);
    push(target, (byte) ((value >> 16) & 0xff), mem);
    push(target, (byte) ((value >> 24) & 0xff), mem);
    push(target, (byte) ((value >> 32) & 0xff), mem);
    push(target, (byte) ((value >> 40) & 0xff), mem);
    push(target, (byte) ((value >> 48) & 0xff), mem);
    push(target, (byte) ((value >> 56) & 0xff), mem);
}

fn(void, __x86_64_linux_machine_emit_dword, i32 value, bytecode target) {
    push(target, (byte) (value & 0xff), mem);
    push(target, (byte) ((value >> 8) & 0xff), mem);
    push(target, (byte) ((value >> 16) & 0xff), mem);
    push(target, (byte) ((value >> 24) & 0xff), mem);
}

// MOV $value, RAX
fn(void, __x86_64_linux_machine_rax, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xb8, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_rbx, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xbb, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_rcx, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xb9, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_rdi, i64 value, bytecode target) {
    // __x86_64_linux_machine_int3(target, mem);
    push(target, 0x48, mem);
    push(target, 0xbf, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_rdi_short, i32 value, bytecode target) {
    // __x86_64_linux_machine_int3(target, mem);
    // xor rdi, rdi
    push(target, 0x48, mem);
    push(target, 0x31, mem);
    push(target, 0xff, mem);
    if (value == 0) return;
    // mov rdi, value
    push(target, 0xbf, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, __x86_64_linux_machine_rsi, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xbe, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_rdx, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xba, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r8, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xb8, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r9, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xb9, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r10, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xba, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r11, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xbb, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r12, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xbc, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r13, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xbd, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r15, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xbf, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_put_pointer, i64 put_in, i64 value, indexes replace_pointers) {
    struct Pointer pointer = { .put_in = put_in, .value = value };
    push(replace_pointers, pointer, mem);
}

fn(void, __x86_64_linux_machine_rdi_in_ptr, i64 ptr, bytecode target, indexes replace_pointers) {
    if (ptr >= 1) {
        // mov rdi, ptr
        // mov rdi, [rdi]
        __x86_64_linux_put_pointer(target->size + 2, ptr - 1, replace_pointers, mem);
        __x86_64_linux_machine_rax(ptr - 1, target, mem);
        push(target, 0x48, mem);
        push(target, 0x89, mem);
        push(target, 0x38, mem);
    } else {
        // mov [_ptr + r15], rdi
        push(target, 0x49, mem);
        push(target, 0x89, mem);
        push(target, 0xbf, mem);
        __x86_64_linux_machine_emit_dword(ptr, target, mem);
    }
}

fn(void, __x86_64_linux_machine_ptr_in_rdi, i64 ptr, bytecode target, indexes replace_pointers) {
    ground();

    // If previous instruction is a push, just remove it and don't add this one.
    var simulated_ptr = (bytecode) new (byte, 0);
    var env           = new (struct x86_64_linux_env);
    var rep_ptrs      = (indexes) new (struct Pointer, 0);
    env->target       = simulated_ptr;
    __x86_64_linux_machine_rdi_in_ptr(ptr, env->target, rep_ptrs, scratch);

    if (target->size > env->target->size) {
        if (strncmp(
                (char *) env->target->array,
                (char *) &target->array[ target->size - env->target->size ],
                env->target->size)
            == 0) {
            release();
            return;
        }
    } else {
        release();
    }

    if (ptr >= 1) {
        // mov rax, ptr
        // mov [rax], rdi
        __x86_64_linux_put_pointer(target->size + 2, ptr - 1, replace_pointers, mem);
        __x86_64_linux_machine_rax(((i64) target->array) + ptr - 1, target, mem);
        push(target, 0x48, mem);
        push(target, 0x89, mem);
        push(target, 0x38, mem);
    } else { // read from offset stack
        // mov rdi, [_ptr + r15]
        push(target, 0x49, mem);
        push(target, 0x8b, mem);
        push(target, 0xbf, mem);
        __x86_64_linux_machine_emit_dword(ptr, target, mem);
    }

    release();
}

fn(void, __x86_64_linux_machine_syscall, bytecode target) {
    // __x86_64_linux_machine_int3(target, mem);
    push(target, 0x0f, mem);
    push(target, 0x05, mem);
}

fn(void, __x86_64_linux_machine_zero_rdi, bytecode target) {
    push(target, 0x48, mem); // same as xor rdi, rdi
    push(target, 0x31, mem);
    push(target, 0xff, mem);
}

fn(void, __x86_64_linux_machine_esi, i64 value, bytecode target) {
    push(target, 0xbe, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, __x86_64_linux_machine_eax, i64 value, bytecode target) {
    push(target, 0xb8, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, __x86_64_linux_machine_edx, i64 value, bytecode target) {
    push(target, 0xba, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r10d, i64 value, bytecode target) {
    push(target, 0x41, mem);
    push(target, 0xba, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r8d, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xc7, mem);
    push(target, 0xc0, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r9d, i64 value, bytecode target) {
    push(target, 0x41, mem);
    push(target, 0xb9, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

// TO BE IMPROVED: Make this calculate offsets ffs!
fn(void, __x86_64_linux_machine_jmp0, i64 where, bytecode target, indexes replace_pointers) {
    // RBX contains the actual jump location, whilst RCX contains the address right after the JMP.
    // cmove moves rcx into rbx when rax is zero.

    var rbx_place = target->size + 2;
    __x86_64_linux_machine_rbx(where, target, mem);

    var rcx_place = target->size + 2;
    __x86_64_linux_machine_rcx(-target->size, target, mem);

    // cmp rdi, 0
    push(target, 0x48, mem);
    push(target, 0x83, mem);
    push(target, 0xff, mem);
    push(target, 0x00, mem);

    // cmovne rcx, rbx
    push(target, 0x48, mem);
    push(target, 0x0f, mem);
    push(target, 0x45, mem);
    push(target, 0xd9, mem);

    // call rbx
    push(target, 0xff, mem);
    push(target, 0xd3, mem);

    __x86_64_linux_put_pointer(rcx_place, target->size, replace_pointers, mem);
    __x86_64_linux_put_pointer(rbx_place, where, replace_pointers, mem);
}

fn(void, __x86_64_linux_machine_jmpn0, i64 where, bytecode target, indexes replace_pointers) {
    // RBX contains the actual jump location, whilst RCX contains the address right after the JMP.

    var rbx_place = target->size + 2;
    __x86_64_linux_machine_rbx(where, target, mem);

    var rcx_place = target->size + 2;
    __x86_64_linux_machine_rcx(where, target, mem);

    // cmp rdi, 0
    push(target, 0x48, mem);
    push(target, 0x83, mem);
    push(target, 0xff, mem);
    push(target, 0x00, mem);

    // cmove rcx, rbx
    push(target, 0x48, mem);
    push(target, 0x0f, mem);
    push(target, 0x44, mem);
    push(target, 0xd9, mem);

    // call rbx
    push(target, 0xff, mem);
    push(target, 0xd3, mem);

    __x86_64_linux_put_pointer(rcx_place, target->size, replace_pointers, mem);
    __x86_64_linux_put_pointer(rbx_place, where, replace_pointers, mem);
}

fn(void, __x86_64_linux_machine_call, i64 where, bytecode target, indexes replace_pointers) {
    var rbx_place = target->size + 2;
    __x86_64_linux_machine_rbx(where, target, mem);

    // call rbx
    push(target, 0xff, mem);
    push(target, 0xd3, mem);

    __x86_64_linux_put_pointer(rbx_place, where, replace_pointers, mem);
}

fn(void, __x86_64_linux_machine_mmap, bytecode target) {
    // addr(null) size(size) prot(0b11) flags(0x22) fd(-1) offset(0)
    // syscall number
    // machine_eax(9, target, mem);
    __x86_64_linux_machine_eax(9, target, mem);
    // address
    // machine_rdi(0, target, mem);
    __x86_64_linux_machine_zero_rdi(target, mem);
    // len
    // machine_esi(size, target, mem);
    // prot
    __x86_64_linux_machine_edx(0b111, target, mem);
    // flags
    __x86_64_linux_machine_r10d(0x22, target, mem);
    // fd
    __x86_64_linux_machine_r8d(-1, target, mem);
    // offset
    __x86_64_linux_machine_r9d(0, target, mem);
}

fn(void, __x86_64_linux_machine_rax_in_rdi, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0xc7, mem);
}

fn(void, __x86_64_linux_machine_rdi_in_rsi, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0xfe, mem);
}

fn(void, __x86_64_linux_machine_munmap, bytecode target) {
    __x86_64_linux_machine_rax(11, target, mem);
    // machine_esi(size, target, mem);
}

// r11 - normal stack
// r13 - normal restore stack
// r15 - variable stack
// r12 - variable restore stack

fn(void, ____x86_64_linux_machine_push_rdi, struct x86_64_linux_env *environment) {
    var target = environment->target;

    // __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stacks.main_stack, target, mem);
    // cmp r11, size
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xfb, mem);
    __x86_64_linux_machine_emit_dword(MAIN_STACK_SIZE / 8, target, mem);
    // cmovg r11, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4f, mem);
    push(target, 0xda, mem);
    // mov [r10 + r11 * 8], rdi
    push(target, 0x4b, mem);
    push(target, 0x89, mem);
    push(target, 0x3c, mem);
    push(target, 0xda, mem);
    // inc r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xc3, mem);
}

fn(void, ____x86_64_linux_machine_pop_rdi, struct x86_64_linux_env *environment) {
    ground();
    var target = environment->target;

    // __x86_64_linux_machine_int3(target, mem);
    // dec r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xcb, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stacks.main_stack, target, mem);
    // cmp r11, -1
    push(target, 0x49, mem);
    push(target, 0x83, mem);
    push(target, 0xfb, mem);
    push(target, 0xff, mem);
    // cmovl r11, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4c, mem);
    push(target, 0xda, mem);
    // mov rdi, [r10 + r11 * 8]
    push(target, 0x4b, mem);
    push(target, 0x8b, mem);
    push(target, 0x3c, mem);
    push(target, 0xda, mem);

    release();
}

fn(void, __x86_64_linux_machine_push_var_rdi, struct x86_64_linux_env *environment) {
    var target = environment->target;

    // __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stacks.main_stack, target, mem);
    // cmp r15, 0
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xff, mem);
    __x86_64_linux_machine_emit_dword(VAR_STACK_SIZE / 8, target, mem);
    // cmovg r15, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4f, mem);
    push(target, 0xfa, mem);
    // mov [r10 + r11 * 8], rdi
    push(target, 0x4b, mem);
    push(target, 0x89, mem);
    push(target, 0x3c, mem);
    push(target, 0xfa, mem);
    // inc r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xc7, mem);
}

fn(void, __x86_64_linux_machine_pop_var_rdi, struct x86_64_linux_env *environment) {
    ground();
    var target = environment->target;

    // If previous instruction is a push, just remove it and don't add this one.
    var simulated_push = (bytecode) new (byte, 0);
    var env            = new (struct x86_64_linux_env);
    env->target        = simulated_push;
    env->stacks        = environment->stacks;
    __x86_64_linux_machine_push_var_rdi(env, scratch);

    if (target->size > env->target->size) {
        if (strncmp(
                (char *) env->target->array,
                (char *) &target->array[ target->size - env->target->size ],
                env->target->size)
            == 0) {
            extend((Array) target, -env->target->size, mem);
            release();
            return;
        }
    } else {
        release();
    }

    // __x86_64_linux_machine_int3(target, mem);
    // dec r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xcf, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stacks.main_stack, target, mem);
    // cmp r11, 0
    push(target, 0x49, mem);
    push(target, 0x83, mem);
    push(target, 0xff, mem);
    push(target, 0x00, mem);
    // cmovl r11, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4c, mem);
    push(target, 0xfa, mem);
    // mov rdi, [r10 + r11 * 8]
    push(target, 0x4b, mem);
    push(target, 0x8b, mem);
    push(target, 0x3c, mem);
    push(target, 0xfa, mem);

    release();
}

fn(void, __x86_64_linux_machine_push_restore_r11, struct x86_64_linux_env *environment) {
    var target = environment->target;

    // __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stacks.restore_main_stack, target, mem);
    // cmp r15, 0
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xfd, mem);
    __x86_64_linux_machine_emit_dword(RESTORE_MAIN_STACK_SIZE / 8, target, mem);
    // cmovg r15, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4f, mem);
    push(target, 0xea, mem);
    // mov [r10 + r15 * 8], r11
    push(target, 0x4f, mem);
    push(target, 0x89, mem);
    push(target, 0x1c, mem);
    push(target, 0xea, mem);
    // inc r15
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xc5, mem);
}

fn(void, __x86_64_linux_machine_pop_restore_r11, struct x86_64_linux_env *environment) {
    ground();
    var target = environment->target;

    // If previous instruction is a push, just remove it and don't add this one.
    var simulated_push = (bytecode) new (byte, 0);
    var env            = new (struct x86_64_linux_env);
    env->target        = simulated_push;
    env->stacks        = environment->stacks;
    __x86_64_linux_machine_push_restore_r11(env, scratch);

    if (target->size > env->target->size) {
        if (strncmp(
                (char *) env->target->array,
                (char *) &target->array[ target->size - env->target->size ],
                env->target->size)
            == 0) {
            extend((Array) target, -env->target->size, mem);
            release();
            return;
        }
    } else {
        release();
    }

    // dec r15
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xcd, mem);
    // __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stacks.restore_main_stack, target, mem);
    // cmp r15, -1
    push(target, 0x49, mem);
    push(target, 0x83, mem);
    push(target, 0xfd, mem);
    push(target, 0xff, mem);
    // cmovl r15, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4c, mem);
    push(target, 0xea, mem);
    // mov r11, [r10 + r15 * 8]
    push(target, 0x4f, mem);
    push(target, 0x8b, mem);
    push(target, 0x1c, mem);
    push(target, 0xea, mem);

    release();
}

// TODO: MAKE THESE GUYS BOUND CHECK!!!

fn(void, __x86_64_linux_machine_extend_r15, i64 offset, struct x86_64_linux_env *environment) {
    var target = environment->target;
    if (offset == 0) return;

    // add r15, _offset
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xc7, mem);
    __x86_64_linux_machine_emit_dword(offset * 8, target, mem);
}

fn(void, __x86_64_linux_machine_retract_r15, i64 offset, struct x86_64_linux_env *environment) {
    ground();
    var target = environment->target;
    if (offset == 0) return;

    // If previous instruction is a push, just remove it and don't add this one.
    var simulated_push = (bytecode) new (byte, 0);
    var env            = new (struct x86_64_linux_env);
    env->target        = simulated_push;
    env->stacks        = environment->stacks;
    __x86_64_linux_machine_extend_r15(offset, env, scratch);

    if (target->size > env->target->size) {
        if (strncmp(
                (char *) env->target->array,
                (char *) &target->array[ target->size - env->target->size ],
                env->target->size)
            == 0) {
            extend((Array) target, -env->target->size, mem);
            release();
            return;
        }
    } else {
        release();
    }

    // sub r15, _offset
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xef, mem);
    __x86_64_linux_machine_emit_dword(offset * 8, target, mem);

    release();
}

fn(void, __x86_64_linux_machine_nop, bytecode target) { push(target, 0x90, mem); }

// Steps:
// 1. [x] Figure out ELF
// 2. [ ] Figure out DATA segment
// 3. [ ] Figure out TEXT segment
// 4. [ ] Rewrite the arena allocator in assembly, then hand-assemmble it to x86
// 5. [ ] Utilize that to perform calls and returns
// 6. [ ] Rewrite every standard function in the language in assembly then hand-assemble it to
// x86 This process is REQUIRED for addrI, addrD, jmp0, jmpn0, call, ret, set, pop and push

fn(void, x86_64_linux_machine_pop_restore_r11, x86_64_linux_env environment) {
    environment->stack_diff = 0;
    __x86_64_linux_machine_call(
        environment->pop_restore_ptr, environment->target, environment->replace_pointers, mem);
}

fn(void, x86_64_linux_machine_push_restore_r11, x86_64_linux_env environment) {
    environment->stack_diff = 0;
    __x86_64_linux_machine_call(
        environment->push_restore_ptr, environment->target, environment->replace_pointers, mem);
}

// RET
fn(void, __x86_64_linux_machine_ret, void *_environment, byte restore_state) {
    var environment = (x86_64_linux_env) _environment;

    if (restore_state) {
        if (environment->cur_block_size > 0) x86_64_linux_machine_pop_restore_r11(environment, mem);
        __x86_64_linux_machine_retract_r15(environment->cur_block_size, environment, mem);
    }

    push(environment->target, 0xc3, mem);
}

// Function to initialize and return an ELF header
Elf64_Ehdr __x86_64_linux_create_elf_header() {
    Elf64_Ehdr header;

    // Initialize the ELF identification
    memset(header.e_ident, 0, EI_NIDENT);
    header.e_ident[ 0 ] = 0x7f; // ELF magic number
    header.e_ident[ 1 ] = 'E';
    header.e_ident[ 2 ] = 'L';
    header.e_ident[ 3 ] = 'F';
    header.e_ident[ 4 ] = 2; // 64-bit architecture
    header.e_ident[ 5 ] = 1; // Little-endian
    header.e_ident[ 6 ] = 1; // ELF version
    header.e_ident[ 7 ] = 0; // OS/ABI
    header.e_ident[ 8 ] = 0; // ABI version
    // Remaining e_ident bytes are zero

    // Set other fields
    header.e_type      = 2;                  // Executable file
    header.e_machine   = 62;                 // x86-64
    header.e_version   = 1;                  // Current version
    header.e_phoff     = sizeof(Elf64_Ehdr); // Program header offset
    header.e_flags     = 0;                  // Processor-specific flags
    header.e_ehsize    = sizeof(Elf64_Ehdr); // ELF header size
    header.e_shoff     = 0;                  // Section header offset (to be set later)
    header.e_entry     = 0;                  // Entry point address (to be set later)
    header.e_phentsize = 0;                  // Size of program header entry (to be set later)
    header.e_phnum     = 0;                  // Number of program header entries (to be set later)
    header.e_shentsize = 0;                  // Size of section header entry (to be set later)
    header.e_shnum     = 0;                  // Number of section header entries (to be set later)
    header.e_shstrndx  = 0;                  // Section header string table index (to be set later)

    return header;
}

fn(void, __x86_64_linux_machine_elf, bytecode target) {
    // Generate ELF header
    var  _elf_header                      = __x86_64_linux_create_elf_header();
    char elf_header[ sizeof(Elf64_Ehdr) ] = { 0 };
    memcpy(elf_header, &_elf_header, sizeof(Elf64_Ehdr));

    for (i32 i = 0; i < sizeof(Elf64_Ehdr); i++) {
        var ch = elf_header[ i ];
        push(target, ch, mem);
    }
}

#define COLUMNS 16
void __x86_64_linux_print_bytecode(i64 size, byte *target) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", target[ i ]);
        if (i % (COLUMNS) == (COLUMNS - 1) && i > 0) printf("\n");
    }
    printf("\n");
}

mc __x86_64_linux_get_exec(byte *target, i64 func_start, i64 size) {
    var executable
        = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(executable, target, size);

    return &executable[ func_start ];
}

i64 __x86_64_linux_get_rax_value() {
    i64 rax_value;

    __asm__("mov %%rax, %0" : "=r"(rax_value) : : "%rax");

    return rax_value;
}

i64 __x86_64_linux_get_r13_value() {
    i64 r13_value;

    __asm__("mov %%r13, %0" : "=r"(r13_value) : : "%rax");

    return r13_value;
}

i64 __x86_64_linux_get_r15_value() {
    i64 r13_value;

    __asm__("mov %%r15, %0" : "=r"(r13_value) : : "%rax");

    return r13_value;
}

i64 __x86_64_linux_get_stack_value() {
    i64 r13_value;

    __asm__("mov %%rsp, %0" : "=r"(r13_value) : : "%rax");

    return r13_value;
}

i64 __x86_64_linux_get_rdi_value() {
    i64 rdi_value;

    __asm__("mov %%rdi, %0" : "=r"(rdi_value) : : "%rax");

    return rdi_value;
}

i64 __x86_64_linux_get_rdx_value() {
    i64 rdx_value;

    __asm__("mov %%rdx, %0" : "=r"(rdx_value) : : "%rax");

    return rdx_value;
}

fn(void, __x86_64_linux_insert_ptr_space, x86_64_linux_env env, bytecode target) {
    var amount = env->ir.segments->array[ env->main_segment ].block->stack_size;
    printf("Needs: %li\n", amount);
    extend((void *) target, 8 * amount, mem);
}

void __x86_64_linux_replace_bytecode_pointers(
    i64 function_offset, byte *target, x86_64_linux_env environment, byte symbolic) {
    var pointers = environment->replace_pointers;

    var destination = &target[ -function_offset ];
    for (i64 i = 0; i < pointers->size; i++) {
        if (pointers->array[ i ].put_in == -1) continue;
        var addr = (i64 *) (&destination[ pointers->array[ i ].put_in ]);

        var value_i = pointers->array[ i ].value;
        var value   = (i64) -1;
        if (value_i < 0) {
            for (i64 i = 0; i < environment->block_locations->size; i++) {
                var cur = environment->block_locations->array[ i ];

                if (cur.alan_name == -value_i) {
                    value = cur.actual_address;

                    break;
                }
            }

            if (value == -1) {
                error(
                    scribe_error,
                    "I (x86_64_linux) was not able to find the referenced block symbol for pointer "
                    "replacement. "
                    "This should not be happening!");
            }
        } else {
            value = value_i;
        }

        // ADD `destination + ` BACK IN
        *addr = (i64) (symbolic ? ((byte *) -function_offset) : (destination)) + value;
    }
}

fn(void, x86_64_linux_useless, void *_environment) {
    sdebug(useless);
    // nothing to do.
}

fn(void, x86_64_linux_cycle, void *_environment) {
    // var target = ((x86_64_linux_env) _environment)->target;
    // push(target, 0x66, mem);
    // push(target, 0x90, mem);
}

fn(void *, x86_64_linux_create_env, IR ir) {
    sdebug(run create env);
    var env               = ret(struct x86_64_linux_env);
    env->block_locations  = (void *) ret(struct block_loc, 0);
    env->target           = (bytecode) ret(char, 0);
    env->replace_pointers = (indexes) ret(struct Pointer, 0);
    env->ir               = ir;

    __x86_64_linux_insert_ptr_space(env, env->target, mem);

    env->func_start = env->target->size;

    env->allocator = 0;

    env->stacks.main_stack = alloc(mem, MAIN_STACK_SIZE * 8);
    // __x86_64_linux_machine_r11(0x0, env->target, mem);

    env->stacks.restore_main_stack = alloc(mem, RESTORE_MAIN_STACK_SIZE * 8);
    // __x86_64_linux_machine_r13(0x0, env->target, mem);

    env->stacks.var_stack = alloc(mem, VAR_STACK_SIZE * 8);
    __x86_64_linux_machine_r15((i64) env->stacks.var_stack, env->target, mem);

    env->put_main = env->target->size + 2;
    __x86_64_linux_machine_rbx(-env->put_main, env->target, mem);

    // jmp rbx (main segment)
    push(env->target, 0xff, mem);
    push(env->target, 0xe3, mem);

    env->jump_after = env->target->size + 2;

    // PUT EXTRA CODE HERE
    env->push_ptr = env->target->size;
    ____x86_64_linux_machine_push_rdi(env, mem);
    __x86_64_linux_machine_ret(env, 0, mem);

    env->pop_ptr = env->target->size;
    ____x86_64_linux_machine_pop_rdi(env, mem);
    __x86_64_linux_machine_ret(env, 0, mem);

    for (i64 i = 0; i < ir.segments->size; i++) {
        if (ir.segments->array[ i ].block == null) continue;
        if (ir.segments->array[ i ].block->stack_size > 0) {
            env->push_restore_ptr = env->target->size;
            __x86_64_linux_machine_push_restore_r11(env, mem);
            __x86_64_linux_machine_ret(env, 0, mem);

            env->pop_restore_ptr = env->target->size;
            __x86_64_linux_machine_pop_restore_r11(env, mem);
            __x86_64_linux_machine_ret(env, 0, mem);

            break;
        }
    }

    // __x86_64_linux_machine_nop(env->target, mem);

    return env;
}

void me_bug() {
    var r13       = __x86_64_linux_get_r13_value();
    var stack_ptr = __x86_64_linux_get_stack_value();
    printf("Called %p stack ptr %li\n", (void *) r13, stack_ptr);
    return;
}

fn(void, x86_64_linux_debug, void *_environment) {
    return;
    var environment = (x86_64_linux_env) _environment;
    var target      = environment->target;

    var ptr = target->size - environment->func_start;
    push(target, 0x49, mem);
    push(target, 0xbd, mem);
    __x86_64_linux_machine_emit_qword((uint64_t) ptr, target, mem);
    push(target, 0x49, mem);
    push(target, 0xbb, mem);
    __x86_64_linux_machine_emit_qword((uint64_t) me_bug, target, mem);
    push(target, 0x41, mem);
    push(target, 0xff, mem);
    push(target, 0xd3, mem);
}

fn(void, x86_64_linux_push_from_tmp, void *_environment) {
    sdebug(push tmp);
    var environment = (x86_64_linux_env) _environment;
    environment->stack_diff++;
    environment->last_push_index = environment->target->size;
    __x86_64_linux_machine_call(
        environment->push_ptr, environment->target, environment->replace_pointers, mem);
}

fn(void, x86_64_linux_pop_to_tmp, void *_environment) {
    ground();
    sdebug(pop tmp);
    var environment = (x86_64_linux_env) _environment;
    var target      = environment->target;
    environment->stack_diff--;

    // If previous instruction is a push, just remove it and don't add this one.
    var simulated_push    = (bytecode) new (byte, 0);
    var env               = new (struct x86_64_linux_env);
    env->replace_pointers = (indexes) new (struct Pointer, 0);
    env->target           = simulated_push;
    env->stacks           = environment->stacks;
    env->push_ptr         = environment->push_ptr;
    x86_64_linux_push_from_tmp(env, mem);

    if (target->size > env->target->size) {
        if (strncmp(
                (char *) env->target->array,
                (char *) &target->array[ target->size - env->target->size ],
                env->target->size)
            == 0) {
            var sim_one = env->replace_pointers->array[ 0 ];

            for (i64 i = 0; i < environment->replace_pointers->size; i++) {
                var rep_ptr = &environment->replace_pointers->array[ i ];
                var dest    = target->size - env->target->size + 2;

                if (dest == rep_ptr->put_in) { rep_ptr->put_in = -1; }
            }

            extend((Array) target, -env->target->size, mem);

            // target->size = environment->last_push_index;
            release();
            return;
        }
    } else {
        release();
    }

    __x86_64_linux_machine_call(environment->pop_ptr, target, environment->replace_pointers, mem);
    // x86_64_linux_debug(environment, mem);
    release();
}

fn(void, x86_64_linux_ret, void *_environment) {
    sdebug(ret);
    ground();
    var environment = (x86_64_linux_env) _environment;
    var target      = environment->target;

    // If previous instruction is a ret, just remove it and don't add this one.
    var simulated_ret = (bytecode) new (byte, 0);
    var env           = new (struct x86_64_linux_env);
    env->target       = simulated_ret;
    env->stacks       = environment->stacks;
    __x86_64_linux_machine_ret(env, 1, scratch);

    // if (target->size > env->target->size) {
    //     if (strncmp(
    //             (char *) env->target->array,
    //             (char *) &target->array[ target->size - env->target->size ],
    //             env->target->size)
    //         == 0) {
    //         release();
    //         return;
    //     }
    // } else {
    //     release();
    // }

    // TODO: Add memory cleanup and persistence after I implement the memory safety
    // Mechanism:
    // 1. Get the parent block that's being returned from the allocator
    // 2. Remove it from the current allocator arena and hand it over to the arena that's on top of
    // us
    // 3. Return the address we need in it
    // This is faster and more efficient than cloning the entire array in the top arena
    __x86_64_linux_machine_ret(environment, 1, mem);
    release();
}

fn(void, x86_64_linux_dryback, void *_environment) {
    sdebug(dryback);
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_ret(environment, 1, mem);
}

fn(void, x86_64_linux_const_to_tmp, i64 value, void *_environment) {
    sdebug(const);
    var environment = (x86_64_linux_env) _environment;
    if (llabs(value) < INT32_MAX) {
        __x86_64_linux_machine_rdi_short(value, environment->target, mem);
    } else {
        __x86_64_linux_machine_rdi(value, environment->target, mem);
    }
}

fn(void, x86_64_linux_jmp0, i64 where, void *_environment) {
    sdebug(jmp zero);
    var environment = (x86_64_linux_env) _environment;

    __x86_64_linux_machine_jmp0(-where, environment->target, environment->replace_pointers, mem);
}

fn(void, x86_64_linux_jmpn0, i64 where, void *_environment) {
    sdebug(jmp not zero);
    var environment = (x86_64_linux_env) _environment;

    __x86_64_linux_machine_jmpn0(-where, environment->target, environment->replace_pointers, mem);
}

fn(i32, __x86_64_linux_calculate_var_offset, i64 sub_top_index, x86_64_linux_env environment) {
    var sub_index = sub_top_index & 0xFFFFFFFF;
    var top_index = sub_top_index >> 32;

    if (top_index == 0) {
        // printf("gvar: %li\n", sub_index);

        return sub_index * 8 + 1;
    }

    i64 total = 0;

    var ptr = environment->ir.segments->array[ environment->cur_block_index ];
    while (ptr.parent != -1 && ptr.name != top_index) {
        ptr = environment->ir.segments->array[ ptr.parent ];
        total -= ptr.block->stack_size;
    }

    total -= sub_index;

    printf("input 0x%lX output %li\n", sub_top_index, total);
    return total * 8;
}

fn(void, x86_64_linux_addr_deref_to_tmp, i64 addr, i64 sub_top_index, void *_environment) {
    sdebug(deref address);
    var environment = (x86_64_linux_env) _environment;
    var target      = environment->target;

    // TODO: Massive bug!! The index does NOT take into account higher scopes!
    __x86_64_linux_machine_ptr_in_rdi(
        __x86_64_linux_calculate_var_offset(sub_top_index, environment, mem),
        target,
        environment->replace_pointers,
        mem);
}

fn(void, x86_64_linux_addr_set_addr_to_tmp, i64 addr, i64 sub_top_index, void *_environment) {
    sdebug(set address);
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_rdi_in_ptr(
        __x86_64_linux_calculate_var_offset(sub_top_index, environment, mem),
        environment->target,
        environment->replace_pointers,
        mem);
}

fn(void, __x86_64_linux_machine_add_short_rdi, i32 value, bytecode target) {
    // mov rdi, r15
    push(target, 0x4c, mem);
    push(target, 0x89, mem);
    push(target, 0xff, mem);

    // add rdi, _value
    push(target, 0x48, mem);
    push(target, 0x81, mem);
    push(target, 0xc7, mem);
    __x86_64_linux_machine_emit_dword(value, target, mem);
}

fn(void, x86_64_linux_create_block, i64 name, IR ir, void *_environment) {
    sdebug(create block);
    var environment      = (x86_64_linux_env) _environment;
    var target           = environment->target;
    var replace_pointers = environment->replace_pointers;

    // for (i64 i = 0; i < environment->stack_diff; i++) {
    //     __x86_64_linux_machine_pop_rdi(target, mem);
    // }
    // environment->stack_diff = 0;

    if (environment->block_locations->size > 0 && target->array[ target->size - 1 ] != 0xc3) {
        __x86_64_linux_machine_ret(environment, 1, mem);
    }

    environment->cur_block_size  = ir.segments->array[ name ].block->stack_size;
    environment->cur_block_index = name;

    // __x86_64_linux_machine_nop(target, mem);

    var location = (struct block_loc) { .actual_address = target->size, .alan_name = name };
    push(environment->block_locations, location, mem);

    var string_name
        = ir.segments->array[ name ].name < 0
              ? "block"
              : ir.compiler_data.context->symbols->array[ ir.segments->array[ name ].name ];

    if (environment->allocator < 0) { environment->allocator = -environment->allocator; }

    if (strcmp(string_name, "alloc") == 0) { environment->allocator = -target->size; }

    if (ir.segments->array[ name ].name == 1) {
        var cur                   = target->size;
        environment->main_segment = ir.segments->array[ name ].block->stack_size;
        environment->main_index   = name;

        __x86_64_linux_put_pointer(environment->put_main, cur, replace_pointers, mem);
    } else {
        if (environment->cur_block_size > 0) {
            x86_64_linux_machine_push_restore_r11(environment, mem);
        }

        __x86_64_linux_machine_extend_r15(environment->cur_block_size, environment, mem);
    }
}

fn(void, __x86_64_linux_efficient_pop, x86_64_linux_env environment) {
    var target = environment->target;

    // call rbx
    push(target, 0xff, mem);
    push(target, 0xd3, mem);
}

// This one's hard to pull off. Really hard.
fn(void, x86_64_linux_call, i64 name, IR ir, void *_environment) {
    sdebug(call func);
    var environment      = (x86_64_linux_env) _environment;
    var target           = environment->target;
    var replace_pointers = environment->replace_pointers;

    var my_name = ir.segments->array[ name ].name;

    var string_name = my_name < 0 ? "block" : ir.compiler_data.context->symbols->array[ my_name ];

    if (strcmp(string_name, "mmap") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);

        __x86_64_linux_machine_mmap(target, mem);
        // get correct i64 value, not a byte-aligned value
        // imul rsi, 8
        push(target, 0x48, mem);
        push(target, 0x6b, mem);
        push(target, 0xf6, mem);
        push(target, 0x08, mem);
        __x86_64_linux_machine_syscall(target, mem);
        __x86_64_linux_machine_rax_in_rdi(target, mem);
    }

    else if (strcmp(string_name, "munmap") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);
        x86_64_linux_pop_to_tmp(environment, mem);
        // hotfix for swapping rdi and rsi
        // xchg rdi, rsi
        push(target, 0x48, mem);
        push(target, 0x87, mem);
        push(target, 0xfe, mem);
        __x86_64_linux_machine_munmap(target, mem);
        // get correct i64 value, not a byte-aligned value
        // imul rsi, 8
        push(target, 0x48, mem);
        push(target, 0x6b, mem);
        push(target, 0xf6, mem);
        push(target, 0x08, mem);
        __x86_64_linux_machine_syscall(target, mem);
    }

    else if (strcmp(string_name, "tmp") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
    }

    else if (strcmp(string_name, "add") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);
        x86_64_linux_pop_to_tmp(environment, mem);

        push(target, 0x48, mem);
        push(target, 0x01, mem);
        push(target, 0xf7, mem);
    }

    else if (strcmp(string_name, "sub") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);
        x86_64_linux_pop_to_tmp(environment, mem);
        push(target, 0x48, mem);
        push(target, 0x29, mem);
        push(target, 0xf7, mem);
    }

    else if (strcmp(string_name, "mul") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);
        x86_64_linux_pop_to_tmp(environment, mem);
        push(target, 0x48, mem);
        push(target, 0x0f, mem);
        push(target, 0xaf, mem);
        push(target, 0xfe, mem);
    }

    else if (strcmp(string_name, "not") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        push(target, 0x48, mem);
        push(target, 0xf7, mem);
        push(target, 0xd7, mem);
    }

    else if (strcmp(string_name, "inctmp") == 0) {
        // inc rdi
        push(target, 0x48, mem);
        push(target, 0xff, mem);
        push(target, 0xc7, mem);
    }

    else if (strcmp(string_name, "getp") == 0) {
        x86_64_linux_pop_to_tmp(environment, mem);
        // mov rdi, [rdi]
        push(target, 0x48, mem);
        push(target, 0x8b, mem);
        push(target, 0x3f, mem);
    }

    else if (strcmp(string_name, "setp") == 0) {
        // value
        x86_64_linux_pop_to_tmp(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);
        // address
        x86_64_linux_pop_to_tmp(environment, mem);
        // mov [rsi], rdi
        push(target, 0x48, mem);
        push(target, 0x89, mem);
        push(target, 0x3e, mem);
    }

    else if (environment->allocator <= 0) {
        __x86_64_linux_machine_call(-name, target, replace_pointers, mem);
    }

    else {
        error(
            scribe_error,
            "The ALA (Alan Allocator) has not yet been defined. The feature you requested cannot "
            "be compiled into the program.");
    }
}

#define awk_patch "awk '{if ($1 ~ /^[0-9a-fA-F]{8}$/) {print \"/*\" $1 \"*/ \" substr($0, 29)}}'"

fn(void, x86_64_linux_finish, void *_environment) {
    sdebug(fin.);
    var environment      = (x86_64_linux_env) _environment;
    var target           = environment->target;
    var replace_pointers = environment->replace_pointers;

    __x86_64_linux_machine_ret(environment, 0, mem);

    var function_pointer
        = __x86_64_linux_get_exec(target->array, environment->func_start, target->size);

    __x86_64_linux_replace_bytecode_pointers(
        environment->func_start, (byte *) function_pointer, environment, 1);

    // -- Uncomment these to write the x86 output to a file allowing for easier debugging --
    var fd = fopen("out/x86_64_linux_dump_symbolic.bin", "w");
    fwrite(function_pointer, target->size - environment->func_start, 1, fd);
    fclose(fd);

    __x86_64_linux_replace_bytecode_pointers(
        environment->func_start, (byte *) function_pointer, environment, 0);

    fd = fopen("out/x86_64_linux_dump.bin", "w");
    fwrite(function_pointer, target->size - environment->func_start, 1, fd);
    fclose(fd);

    // system("ndisasm -b 64 out/x86_64_linux_dump.bin");
    // printf("\n");
    // system("ndisasm -b 64 out/x86_64_linux_dump_symbolic.bin | " awk_patch);
    fd = fopen("out/x86_64_linux_dump.asm", "w");
    fprintf(fd, "; start: %p\n", function_pointer);
    fclose(fd);
    system("ndisasm -b 64 out/x86_64_linux_dump.bin | " awk_patch " >> out/x86_64_linux_dump.asm");
    system("ndisasm -b 64 out/x86_64_linux_dump_symbolic.bin | " awk_patch
           " > out/x86_64_linux_dump_symbolic.asm");

    printf("\n");

    // __x86_64_linux_print_bytecode(target->size, target->array);

    printf("STACK: %p\n", environment->stacks.main_stack);
    printf("VAR: %p\n", environment->stacks.var_stack);
    printf("PLACE: %p\n", function_pointer);
    function_pointer();
    var value = __x86_64_linux_get_r15_value();
    // __x86_64_linux_machine_pop_rdi(environment, mem);
    var rdi = (i64 *) __x86_64_linux_get_rdi_value();
    // __x86_64_linux_print_bytecode(target->size, target->array);
    __x86_64_linux_print_bytecode(
        target->size, (void *) &((byte *) function_pointer)[ -environment->func_start ]);
    printf("\n");
    printf("rdi = %p\n", rdi);
    printf("r15 = %p\n", (void *) value);
    // *rdi = 2;
    printf("size = %li\n", target->size);
    // printf("*rdi = %li\n", *rdi);
}

fn(void, x86_64_linux_test_scribe, IR ir, void *_environment) {
    sdebug(run test scribe);
    var environment      = (x86_64_linux_env) _environment;
    var target           = environment->target;
    var replace_pointers = environment->replace_pointers;
    ground();

    // Used for separating preprocessing values

    // // -- Uncomment these to write the x86 output to a file allowing for easier debugging --
    // var fd = fopen("x86_64_linux_dump.bin", "w");
    // fwrite(function_pointer, target->size - func_start, 1, fd);
    // fclose(fd);

    // print_bytecode(target->size, target->array);
    // printf("\n");

    __x86_64_linux_print_bytecode(target->size, target->array);
    printf("\n");

    // function_pointer();
    // int *rax_value = (int *) __x86_64_linux_get_rax_value();
    // int *rdi_value = (int *) __x86_64_linux_get_rdi_value();

    // __x86_64_linux_print_bytecode(target->size, &((byte *) function_pointer)[ -func_start ]);

    // printf("\n");
    // printf("function signature: %p - %li\n", function_pointer, (i64) rdi_value);

    release();
}

// NOTE: This scribe uses RDI as a temporary register, r10 as the stack pointer, r11 as the stack
// index, r13 as the stack restore index and r15 as the variable stack index.

// TODO: Implement stack_size in INST_LIST!! VERY IMPORTANT!! (Done I think?)
// TODO: Fix returns and drybacks in a block statement

scribe get_scribe_x86_64_linux() {
    var me = (scribe) { //
                        .tests      = x86_64_linux_test_scribe,
                        .create_env = x86_64_linux_create_env,
                        .useless    = x86_64_linux_useless,
                        .nop        = x86_64_linux_useless,
                        .pop_tmp    = x86_64_linux_pop_to_tmp,
                        .push_tmp   = x86_64_linux_push_from_tmp,
                        .ret_tmp    = x86_64_linux_ret,
                        .dryback    = x86_64_linux_dryback,
                        .constant   = x86_64_linux_const_to_tmp,
                        // .plain_addr = x86_64_linux_addr_plain_to_tmp,
                        .deref_addr = x86_64_linux_addr_deref_to_tmp,
                        .set        = x86_64_linux_addr_set_addr_to_tmp,
                        .jmp0       = x86_64_linux_jmp0,
                        .jmpn0      = x86_64_linux_jmpn0,
                        .block      = x86_64_linux_create_block,
                        .call       = x86_64_linux_call,
                        .cycle  = x86_64_linux_cycle, // i don't have a use for this as of now...
                        .finish = x86_64_linux_finish
    };

    return me;
}
