#include "../macros.h"
#include "../types.h"
#include "../utils.h"

//

#include "template.c"

/*
 * Scribe functionality checklist! (x86_64_linux)
 * [x] useless
 * [x] nop
 * [x] pop_to_tmp
 * [x] push_from_tmp
 * [ ] call
 * [ ] ret
 * [ ] jmp0
 * [ ] jmpn0
 * [ ] const_to_tmp
 * [ ] addr_deref_to_tmp
 * [ ] addr_no_deref_to_tmp
 * [ ] addr_set_addr_to_tmp
 * [ ] create_block
 * [ ] run_tests
 * [x] create_env
 */

// [ ] create_block();
// [x] pop(mov [cur_stack], %rex);
// [x] push(push_stack %rex);
// [ ] call(complicated...);
// [ ] ret(complicated...);
// [ ] jmp0(cmp %rex; jmp0 [addr]);
// [ ] jmpn0(reverse of jmp0)
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

    // jmp rbx
    push(target, 0xff, mem);
    push(target, 0xe3, mem);

    struct Pointer rcx_dex = { .put_in = rcx_place, .value = target->size };
    push(replace_pointers, rcx_dex, mem);

    struct Pointer rbx_dex = { .put_in = rbx_place, .value = where };
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

    push(target, 0xff, mem);
    push(target, 0xe3, mem);

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

fn(void, __x86_64_linux_machine_munmap, bytecode target) {
    __x86_64_linux_machine_rax(11, target, mem);
    // machine_esi(size, target, mem);
}

fn(void, __x86_64_linux_machine_push_rdi, bytecode target) { push(target, 0x57, mem); }
fn(void, __x86_64_linux_machine_pop_rdi, bytecode target) { push(target, 0x5f, mem); }

// Steps:
// 1. [x] Figure out ELF
// 2. [ ] Figure out DATA segment
// 3. [ ] Figure out TEXT segment
// 4. [ ] Rewrite the arena allocator in assembly, then hand-assemmble it to x86
// 5. [ ] Utilize that to perform calls and returns
// 6. [ ] Rewrite every standard function in the language in assembly then hand-assemble it to
// x86 This process is REQUIRED for addrI, addrD, jmp0, jmpn0, call, ret, set, pop and push

// RET
fn(void, __x86_64_linux_machine_ret, bytecode target) { push(target, 0xc3, mem); }

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
        if (i % 8 == 7 && i > 0) printf("\n");
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
    for (i32 i = 0; i < ir.compiler_data->reserves->size; i++) { extend((void *) target, 8, mem); }
}

void __x86_64_linux_replace_bytecode_pointers(
    byte *target, i64 function_offset, i64 st_st_target, indexes pointers) {
    var destination = &target[ -function_offset ];
    for (i64 i = 0; i < pointers->size; i++) {
        var addr = (i64 *) (&destination[ pointers->array[ i ].put_in ]);
        *addr    = (i64) destination + pointers->array[ i ].value + st_st_target;
    }
}

typedef struct x86_64_linux_env {
    A(i64) * block_locations;
    bytecode target;
} *x86_64_linux_env;

fn(void, x86_64_linux_useless, void *_environment) {
    // nothing to do.
}

fn(void, x86_64_linux_test_scribe, IR ir, void *_environment) {
    var environment = (x86_64_linux_env) _environment;
    var target = environment->target;
    ground();

    var replace_pointers = (indexes) new (struct Pointer, 0);

    // Used for separating preprocessing values
    var st_st_target = target->size;
    __x86_64_linux_insert_ptr_space(ir, target, scratch);

    var func_start = target->size;

    {
        // put rax in pointer, put 32 in rax and restore rax from pointer

        // machine_rax_in_ptr(0x0, target, (void *) replace_pointers, st_st_target, scratch);
        // machine_rax(32, target, scratch);
        // machine_ptr_in_rax(0x0, target, (void *) replace_pointers, st_st_target, scratch);
    }

    {
        // mmap, put ptr in memory, unmap it, put it back into rax and return

        __x86_64_linux_machine_mmap(target, scratch);
        __x86_64_linux_machine_esi(32, target, scratch);
        __x86_64_linux_machine_syscall(target, scratch);

        // machine_push_rax(target, scratch);
        __x86_64_linux_machine_rax_in_ptr(0x0, target, replace_pointers, scratch);

        __x86_64_linux_machine_rsi(32, target, scratch);
        __x86_64_linux_machine_ptr_in_rdi(0x0, target, replace_pointers, scratch);
        // machine_pop_rax(target, scratch);
        // machine_rax_in_rdi(target, scratch);
        __x86_64_linux_machine_munmap(target, scratch);

        // Uncomment to unmap the allocated memory and therefore causing a segfault when
        // `*rax_value = 2;` is ran.
        __x86_64_linux_machine_syscall(target, scratch);
        __x86_64_linux_machine_ptr_in_rdi(0x0, target, replace_pointers, scratch);
    }

    {
        // while true loop, hopefully...

        // machine_jmp0(target->size, target, replace_pointers, scratch);
    }

    __x86_64_linux_machine_ret(target, scratch);

    var function_pointer = __x86_64_linux_get_exec(target->array, func_start, target->size);

    __x86_64_linux_replace_bytecode_pointers(
        (byte *) function_pointer, func_start, st_st_target, replace_pointers);

    // // -- Uncomment these to write the x86 output to a file allowing for easier debugging --
    // var fd = fopen("x86_64_linux_dump.bin", "w");
    // fwrite(function_pointer, target->size - func_start, 1, fd);
    // fclose(fd);

    // print_bytecode(target->size, target->array);
    // printf("\n");

    __x86_64_linux_print_bytecode(target->size, target->array);
    printf("\n");

    function_pointer();
    int *rax_value = (int *) __x86_64_linux_get_rax_value();
    int *rdi_value = (int *) __x86_64_linux_get_rdi_value();

    __x86_64_linux_print_bytecode(target->size, &((byte *) function_pointer)[ -func_start ]);

    printf("\n");
    printf("function signature: %p - %li\n", function_pointer, (i64) rdi_value);

    printf("\nRDI: %p\n", rdi_value);
    printf("\n" red("ATTENTION! A SIGSEGV IS EXPECTED BEHAVIOR:") "\n");

    *rdi_value = 2;

    printf("[RDI]: %i\n", *rdi_value);

    release();
}

fn(void *, x86_64_linux_create_env, IR ir) {
    var env              = ret(struct x86_64_linux_env);
    env->block_locations = (void *) ret(i64, 0);
    env->target = (bytecode) ret(char, 0);

    return env;
}

fn(void, x86_64_linux_pop_to_tmp, void *_environment) {
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_pop_rdi(environment->target, mem);
}

fn(void, x86_64_linux_push_from_tmp, void *_environment) {
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_push_rdi(environment->target, mem);
}

fn(void, x86_64_linux_ret, void *_environment) {
    var environment = (x86_64_linux_env) _environment;
    // TODO: Add memory cleanup and persistence after I implement the memory safety
    // Mechanism:
    // 1. Get the parent block that's being returned from the allocator
    // 2. Remove it from the current allocator arena and hand it over to the arena that's on top of
    // us
    // 3. Return the address we need in it
    // This is faster and more efficient than cloning the entire array in the top arena
    __x86_64_linux_machine_ret(environment->target, mem);
}

fn(void, x86_64_linux_const_to_tmp, i64 value, void *_environment) {
    var environment = (x86_64_linux_env) _environment;
    __x86_64_linux_machine_rdi(value, environment->target, mem);
}

// NOTE: This scribe uses RDI as a temporary register.

scribe get_scribe_x86_64_linux() {
    var me = (scribe) { //
                        .tests      = x86_64_linux_test_scribe,
                        .create_env = x86_64_linux_create_env,
                        .useless    = x86_64_linux_useless,
                        .nop        = x86_64_linux_useless,
                        .pop        = x86_64_linux_pop_to_tmp,
                        .push       = x86_64_linux_push_from_tmp,
                        .ret        = x86_64_linux_ret,
                        .constant   = x86_64_linux_const_to_tmp,
                        .plain_addr = x86_64_linux_const_to_tmp
    };

    return me;
}