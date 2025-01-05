#include "../macros.h"
#include "../types.h"
#include "../utils.h"

//

#include "template.c"

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

#define STACK_SIZE         ((1 << 13) * sizeof(i64))
#define RESTORE_STACK_SIZE ((1 << 8) * sizeof(i64))
typedef struct x86_64_linux_env {
    A(struct block_loc) * block_locations;
    indexes  replace_pointers;
    bytecode target;
    IR       ir;

    i64 func_start;
    i64 put_main;

    i64 main_segment; // SET AFTER INIT
    i64 allocator;    // SET AFTER INIT

    i64 stack_diff;

    i64 *stack;
    i64 *restore_stack;
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
    push(target, 0x48, mem);
    push(target, 0xbf, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
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

fn(void, __x86_64_linux_machine_r10, i64 value, bytecode target) {
    push(target, 0x49, mem);
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

fn(void, __x86_64_linux_machine_r11, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xbb, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

fn(void, __x86_64_linux_machine_r13, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xbd, mem);
    __x86_64_linux_machine_emit_qword(value, target, mem);
}

// Puts a pointer inside rbx.
fn(void,
   __x86_64_linux_machine_put_pointer,
   struct Pointer ptr,
   bytecode       target,
   indexes        replace_pointers) {
    push(replace_pointers, ptr, mem);
    __x86_64_linux_machine_rbx(0, target, mem);
}

fn(void, __x86_64_linux_machine_rax_in_ptr, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    __x86_64_linux_machine_put_pointer(our_index, target, replace_pointers, mem);
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0x03, mem);
}

fn(void, __x86_64_linux_machine_ptr_in_rax, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    push(replace_pointers, our_index, mem);
    __x86_64_linux_machine_rax(0x0, target, mem);
    push(target, 0x48, mem);
    push(target, 0x8b, mem);
    push(target, 0x00, mem);
}

fn(void, __x86_64_linux_machine_rdi_in_ptr, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    __x86_64_linux_machine_put_pointer(our_index, target, replace_pointers, mem);
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0x3b, mem);
}

fn(void, __x86_64_linux_machine_ptr_in_rdi, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    push(replace_pointers, our_index, mem);
    __x86_64_linux_machine_rdi(0x0, target, mem);
    push(target, 0x48, mem);
    push(target, 0x8b, mem);
    push(target, 0x3f, mem);
}

fn(void, __x86_64_linux_machine_syscall, bytecode target) {
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
    __x86_64_linux_machine_rbx(0, target, mem);

    var rcx_place = target->size + 2;
    __x86_64_linux_machine_rcx(0, target, mem);

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

    struct Pointer rcx_dex = { .put_in = rcx_place, .value = target->size };
    push(replace_pointers, rcx_dex, mem);

    struct Pointer rbx_dex = { .put_in = rbx_place, .value = where };
    push(replace_pointers, rbx_dex, mem);
}

fn(void, __x86_64_linux_machine_call, i64 where, bytecode target, indexes replace_pointers) {
    var rbx_place = target->size + 2;
    __x86_64_linux_machine_rbx(0, target, mem);

    // call rbx
    push(target, 0xff, mem);
    push(target, 0xd3, mem);

    struct Pointer rbx_dex = { .put_in = rbx_place, .value = where + 1 };
    push(replace_pointers, rbx_dex, mem);
}

fn(void, __x86_64_linux_machine_jmpn0, i64 where, bytecode target, indexes replace_pointers) {
    // RBX contains the actual jump location, whilst RCX contains the address right after the JMP.

    var rbx_place = target->size + 2;
    __x86_64_linux_machine_rbx(0, target, mem);

    var rcx_place = target->size + 2;
    __x86_64_linux_machine_rcx(0, target, mem);

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

    struct Pointer rcx_dex = { .put_in = rcx_place, .value = target->size };
    push(replace_pointers, rcx_dex, mem);

    struct Pointer rbx_dex = { .put_in = rbx_place, .value = where };
    push(replace_pointers, rbx_dex, mem);
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

fn(void, __x86_64_linux_machine_int3, bytecode target) {
    return;
    push(target, 0xcd, mem);
    push(target, 0x03, mem);
}

fn(void, __x86_64_linux_machine_push_rdi, struct x86_64_linux_env *environment) {
    var target = environment->target;

    __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stack, target, mem);
    // cmp r11, 0
    push(target, 0x49, mem);
    push(target, 0x83, mem);
    push(target, 0xfb, mem);
    push(target, 0x00, mem);
    // cmovl r11, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4c, mem);
    push(target, 0xda, mem);
    // mov [r10 + r11 * 8], rdi
    push(target, 0x4b, mem);
    push(target, 0x89, mem);
    push(target, 0x3c, mem);
    push(target, 0xda, mem);
    // dec r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xcb, mem);
}

fn(void, __x86_64_linux_machine_pop_rdi, struct x86_64_linux_env *environment) {
    var target = environment->target;

    __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->stack, target, mem);
    // cmp r11, STACK_SIZE
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xfb, mem);
    __x86_64_linux_machine_emit_dword(STACK_SIZE, target, mem);
    // cmovg r11, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4f, mem);
    push(target, 0xda, mem);
    // mov rdi, [r10 + r11 * 8]
    push(target, 0x4b, mem);
    push(target, 0x8b, mem);
    push(target, 0x3c, mem);
    push(target, 0xda, mem);
    // inc r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xc3, mem);
}

fn(void, __x86_64_linux_machine_push_restore_r11, struct x86_64_linux_env *environment) {
    var target = environment->target;

    __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->restore_stack, target, mem);
    // cmp r13, 0
    push(target, 0x49, mem);
    push(target, 0x83, mem);
    push(target, 0xfd, mem);
    push(target, 0x00, mem);
    // cmovl r13, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4c, mem);
    push(target, 0xea, mem);
    // mov [r10 + r13 * 8], rdi
    push(target, 0x4f, mem);
    push(target, 0x89, mem);
    push(target, 0x3c, mem);
    push(target, 0xea, mem);
    // dec r13
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xcd, mem);
}

fn(void, __x86_64_linux_machine_pop_restore_r11, struct x86_64_linux_env *environment) {
    var target = environment->target;

    __x86_64_linux_machine_int3(target, mem);
    // mov r10, stack_ptr
    __x86_64_linux_machine_r10((uint64_t) environment->restore_stack, target, mem);
    // cmp r13, RESTORE_STACK_SIZE
    push(target, 0x49, mem);
    push(target, 0x81, mem);
    push(target, 0xfd, mem);
    __x86_64_linux_machine_emit_dword(RESTORE_STACK_SIZE, target, mem);
    // cmovg r13, r10
    push(target, 0x4d, mem);
    push(target, 0x0f, mem);
    push(target, 0x4f, mem);
    push(target, 0xea, mem);
    // mov rdi, [r10 + r11 * 8]
    push(target, 0x4f, mem);
    push(target, 0x8b, mem);
    push(target, 0x3c, mem);
    push(target, 0xea, mem);
    // inc r11
    push(target, 0x49, mem);
    push(target, 0xff, mem);
    push(target, 0xc5, mem);
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

// RET
fn(void, __x86_64_linux_machine_ret, void *_environment, byte restore_state) {
    var environment = (x86_64_linux_env) _environment;
    if (restore_state) __x86_64_linux_machine_pop_restore_r11(environment, mem);
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

void __x86_64_linux_print_bytecode(i64 size, byte *target) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", target[ i ]);
        if (i % 16 == 15 && i > 0) printf("\n");
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

fn(void, __x86_64_linux_insert_ptr_space, IR ir, bytecode target) {
    printf("Needs: %li\n", ir.compiler_data.reserves->size);
    for (i32 i = 0; i < ir.compiler_data.reserves->size; i++) { extend((void *) target, 8, mem); }
}

void __x86_64_linux_replace_bytecode_pointers(
    byte *target, i64 function_offset, indexes pointers, byte symbolic) {
    var destination = &target[ -function_offset ];
    for (i64 i = 0; i < pointers->size; i++) {
        var addr = (i64 *) (&destination[ pointers->array[ i ].put_in ]);
        // ADD `destination + ` BACK IN
        *addr = (i64) (symbolic ? ((byte *) -function_offset - 1) : (destination - 1))
                + pointers->array[ i ].value;
    }
}

fn(void, x86_64_linux_useless, void *_environment) {
    sdebug(useless);
    // nothing to do.
}

const var JMP_SIZE = 8 /* qword */ + 2 /* mov rbx, value */ + 2 /* jmp rbx */;

fn(void *, x86_64_linux_create_env, IR ir) {
    sdebug(run create env);
    var env               = ret(struct x86_64_linux_env);
    env->block_locations  = (void *) ret(struct block_loc, 0);
    env->target           = (bytecode) ret(char, 0);
    env->replace_pointers = (indexes) ret(struct Pointer, 0);

    __x86_64_linux_insert_ptr_space(ir, env->target, mem);

    env->func_start = env->target->size;

    env->allocator = 0;

    env->stack = alloc(mem, STACK_SIZE);
    __x86_64_linux_machine_r11(0, env->target, mem);

    env->restore_stack = alloc(mem, RESTORE_STACK_SIZE);
    __x86_64_linux_machine_r13(0, env->target, mem);

    env->put_main = env->target->size + 2;
    __x86_64_linux_machine_rbx(0, env->target, mem);

    // jmp rbx (main segment)
    push(env->target, 0xff, mem);
    push(env->target, 0xe3, mem);

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

fn(void, x86_64_linux_pop_to_tmp, void *_environment) {
    sdebug(pop tmp);
    var environment = (x86_64_linux_env) _environment;
    environment->stack_diff--;
    __x86_64_linux_machine_pop_rdi(environment, mem);

    x86_64_linux_debug(environment, mem);
}

fn(void, x86_64_linux_push_from_tmp, void *_environment) {
    sdebug(push tmp);
    var environment = (x86_64_linux_env) _environment;
    environment->stack_diff++;
    __x86_64_linux_machine_push_rdi(environment, mem);
}

fn(void, x86_64_linux_ret, void *_environment) {
    sdebug(ret);
    var environment = (x86_64_linux_env) _environment;
    // TODO: Add memory cleanup and persistence after I implement the memory safety
    // Mechanism:
    // 1. Get the parent block that's being returned from the allocator
    // 2. Remove it from the current allocator arena and hand it over to the arena that's on top of
    // us
    // 3. Return the address we need in it
    // This is faster and more efficient than cloning the entire array in the top arena
    __x86_64_linux_machine_ret(environment, 1, mem);
}

fn(void, x86_64_linux_dryback, void *_environment) {
    sdebug(dryback);
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_ret(environment, 1, mem);
}

fn(void, x86_64_linux_const_to_tmp, i64 value, void *_environment) {
    sdebug(const);
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_rdi(value, environment->target, mem);
}

fn(void, x86_64_linux_jmp0, i64 where, void *_environment) {
    sdebug(jmp zero);
    var environment = (x86_64_linux_env) _environment;

    for (i64 i = 0; i < environment->block_locations->size; i++) {
        var cur = environment->block_locations->array[ i ];

        if (cur.alan_name == where) {
            __x86_64_linux_machine_jmp0(
                cur.actual_address, environment->target, environment->replace_pointers, mem);
            return;
        }
    }

    error(
        scribe_error,
        "I (x86_64_linux) was not able to find the referenced block symbol for jmp0. "
        "This should not be happening!");
}

fn(void, x86_64_linux_jmpn0, i64 where, void *_environment) {
    sdebug(jmp not zero);
    var environment = (x86_64_linux_env) _environment;

    for (i64 i = 0; i < environment->block_locations->size; i++) {
        var cur = environment->block_locations->array[ i ];

        if (cur.alan_name == where) {
            __x86_64_linux_machine_jmpn0(
                cur.actual_address, environment->target, environment->replace_pointers, mem);
            return;
        }
    }

    error(
        scribe_error,
        "I (x86_64_linux) was not able to find the referenced block symbol for jmpn0. "
        "This should not be happening!");
}

fn(void, x86_64_linux_addr_deref_to_tmp, i64 addr, void *_environment) {
    sdebug(deref address);
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_ptr_in_rdi(
        addr, environment->target, environment->replace_pointers, mem);
}

fn(void, x86_64_linux_addr_set_addr_to_tmp, i64 addr, void *_environment) {
    sdebug(set address);
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_rdi_in_ptr(
        addr, environment->target, environment->replace_pointers, mem);
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

    if (environment->block_locations->size > 0 && target->array[ target->size - 1 ] != 0xc3)
        __x86_64_linux_machine_ret(environment, 0, mem);
    __x86_64_linux_machine_nop(target, mem);

    var location = (struct block_loc) { .actual_address = target->size, .alan_name = name };
    push(environment->block_locations, location, mem);

    // for (i64 i = 0; i < environment->block_locations->size; i++) {
    //     printf(
    //         "[CHECK] NAME %li PLACE %li\n",
    //         environment->block_locations->array[ i ].alan_name,
    //         environment->block_locations->array[ i ].actual_address);
    // }

    var string_name
        = ir.segments->array[ name ].name < 0
              ? "block"
              : ir.compiler_data.context->symbols->array[ ir.segments->array[ name ].name ];

    if (environment->allocator < 0) { environment->allocator = -environment->allocator; }

    if (strcmp(string_name, "alloc") == 0) { environment->allocator = -target->size; }

    if (ir.segments->array[ name ].name == 1) {
        var cur                   = target->size;
        environment->main_segment = cur;

        struct Pointer rbx_dex = { .put_in = environment->put_main, .value = cur };
        push(replace_pointers, rbx_dex, mem);
    } else {
        __x86_64_linux_machine_push_restore_r11(environment, mem);
    }
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
        __x86_64_linux_machine_pop_rdi(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);

        __x86_64_linux_machine_mmap(target, mem);
        __x86_64_linux_machine_syscall(target, mem);
    } else if (strcmp(string_name, "munmap") == 0) {
        __x86_64_linux_machine_pop_rdi(environment, mem);
        __x86_64_linux_machine_rdi_in_rsi(target, mem);
        __x86_64_linux_machine_munmap(target, mem);
        __x86_64_linux_machine_syscall(target, mem);
    } else if (environment->allocator <= 0) {
        var loc = -1;
        for (i64 i = 0; i < environment->block_locations->size; i++) {
            var block = environment->block_locations->array[ i ];
            if (block.alan_name != name) continue;
            loc = block.actual_address;
            break;
        }

        if (loc == -1) {
            printf("Name: %lX\n", name);
            error(
                scribe_error,
                "I (x86_64_linux) was not able to find the given call destination."
                "This should not be happening!");
        }

        __x86_64_linux_machine_call(loc, target, replace_pointers, mem);
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
        (byte *) function_pointer, environment->func_start, replace_pointers, 1);

    // -- Uncomment these to write the x86 output to a file allowing for easier debugging --
    var fd = fopen("out/x86_64_linux_dump_symbolic.bin", "w");
    fwrite(function_pointer, target->size - environment->func_start, 1, fd);
    fclose(fd);

    __x86_64_linux_replace_bytecode_pointers(
        (byte *) function_pointer, environment->func_start, replace_pointers, 0);

    fd = fopen("out/x86_64_linux_dump.bin", "w");
    fwrite(function_pointer, target->size - environment->func_start, 1, fd);
    fclose(fd);

    system("ndisasm -b 64 out/x86_64_linux_dump.bin");
    printf("\n");
    system("ndisasm -b 64 out/x86_64_linux_dump_symbolic.bin | " awk_patch);
    fd = fopen("out/x86_64_linux_dump.asm", "w");
    fprintf(fd, "; start: %p\n", function_pointer);
    fclose(fd);
    system("ndisasm -b 64 out/x86_64_linux_dump.bin | " awk_patch " >> out/x86_64_linux_dump.asm");
    system("ndisasm -b 64 out/x86_64_linux_dump_symbolic.bin | " awk_patch
           " > out/x86_64_linux_dump_symbolic.asm");

    printf("\n");
    printf("STACK: %p\n", environment->stack);
    printf("PLACE: %p\n\n", function_pointer);
    function_pointer();
    // __x86_64_linux_machine_pop_rdi(environment, mem);
    var rdi = (byte *) __x86_64_linux_get_rdi_value();
    // __x86_64_linux_print_bytecode(target->size, target->array);
    __x86_64_linux_print_bytecode(
        target->size, (void *) &((byte*)function_pointer)[-environment->func_start]);
    printf("\n");
    printf("RDI %p\n", rdi);
    *rdi = 2;
    printf("WOW\n");
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

// NOTE: This scribe uses RDI as a temporary register.

// TODO: Make create_block add a return statement whenever a block ends (a new one begins)

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
                        .plain_addr = x86_64_linux_const_to_tmp,
                        .jmp0       = x86_64_linux_jmp0,
                        .jmpn0      = x86_64_linux_jmpn0,
                        .deref_addr = x86_64_linux_addr_deref_to_tmp,
                        .set        = x86_64_linux_addr_set_addr_to_tmp,
                        .block      = x86_64_linux_create_block,
                        .call       = x86_64_linux_call,
                        .cycle  = x86_64_linux_useless, // i don't have a use for this as of now...
                        .finish = x86_64_linux_finish
    };

    return me;
}