#include "../macros.h"
#include "../types.h"
#include "../utils.h"

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

fn(void, machine_emit_qword, i64 value, bytecode target) {
    push(target, (byte) (value & 0xff), mem);
    push(target, (byte) ((value >> 8) & 0xff), mem);
    push(target, (byte) ((value >> 16) & 0xff), mem);
    push(target, (byte) ((value >> 24) & 0xff), mem);
    push(target, (byte) ((value >> 32) & 0xff), mem);
    push(target, (byte) ((value >> 40) & 0xff), mem);
    push(target, (byte) ((value >> 48) & 0xff), mem);
    push(target, (byte) ((value >> 56) & 0xff), mem);
}

fn(void, machine_emit_dword, i32 value, bytecode target) {
    push(target, (byte) (value & 0xff), mem);
    push(target, (byte) ((value >> 8) & 0xff), mem);
    push(target, (byte) ((value >> 16) & 0xff), mem);
    push(target, (byte) ((value >> 24) & 0xff), mem);
}

// MOV $value, RAX
fn(void, machine_rax, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xb8, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_rbx, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xbb, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_rcx, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xb9, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_rdi, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xbf, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_rsi, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xbe, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_rdx, i64 value, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0xba, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_r10, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xba, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_r8, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xb8, mem);
    machine_emit_qword(value, target, mem);
}

fn(void, machine_r9, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xb9, mem);
    machine_emit_qword(value, target, mem);
}

// Puts a pointer inside rbx.
fn(void, machine_put_pointer, struct Pointer ptr, bytecode target, indexes replace_pointers) {
    push(replace_pointers, ptr, mem);
    machine_rbx(0, target, mem);
}

fn(void, machine_rax_in_ptr, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    machine_put_pointer(our_index, target, replace_pointers, mem);
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0x03, mem);
}

fn(void, machine_ptr_in_rax, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    push(replace_pointers, our_index, mem);
    machine_rax(0x0, target, mem);
    push(target, 0x48, mem);
    push(target, 0x8b, mem);
    push(target, 0x00, mem);
}

fn(void, machine_rdi_in_ptr, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    machine_put_pointer(our_index, target, replace_pointers, mem);
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0x3b, mem);
}

fn(void, machine_ptr_in_rdi, i64 ptr, bytecode target, indexes replace_pointers) {
    struct Pointer our_index = { .put_in = target->size + 2, .value = ptr * 8 };
    push(replace_pointers, our_index, mem);
    machine_rdi(0x0, target, mem);
    push(target, 0x48, mem);
    push(target, 0x8b, mem);
    push(target, 0x3f, mem);
}

fn(void, machine_syscall, bytecode target) {
    push(target, 0x0f, mem);
    push(target, 0x05, mem);
}

fn(void, machine_zero_rdi, bytecode target) {
    push(target, 0x48, mem); // same as xor rdi, rdi
    push(target, 0x31, mem);
    push(target, 0xff, mem);
}

fn(void, machine_esi, i64 value, bytecode target) {
    push(target, 0xbe, mem);
    machine_emit_dword(value, target, mem);
}

fn(void, machine_eax, i64 value, bytecode target) {
    push(target, 0xb8, mem);
    machine_emit_dword(value, target, mem);
}

fn(void, machine_edx, i64 value, bytecode target) {
    push(target, 0xba, mem);
    machine_emit_dword(value, target, mem);
}

fn(void, machine_r10d, i64 value, bytecode target) {
    push(target, 0x41, mem);
    push(target, 0xba, mem);
    machine_emit_dword(value, target, mem);
}

fn(void, machine_r8d, i64 value, bytecode target) {
    push(target, 0x49, mem);
    push(target, 0xc7, mem);
    push(target, 0xc0, mem);
    machine_emit_dword(value, target, mem);
}

fn(void, machine_r9d, i64 value, bytecode target) {
    push(target, 0x41, mem);
    push(target, 0xb9, mem);
    machine_emit_dword(value, target, mem);
}

// TO BE IMPROVED: Make this calculate offsets ffs!
fn(void, machine_jmp0, i64 where, bytecode target, indexes replace_pointers) {
    // RBX contains the actual jump location, whilst RCX contains the address right after the JMP.
    // cmove moves rcx into rbx when rax is zero.

    var rbx_place = target->size + 2;
    machine_rbx(0, target, mem);

    var rcx_place = target->size + 2;
    machine_rcx(0, target, mem);

    // cmp rax, 0
    push(target, 0x48, mem);
    push(target, 0x83, mem);
    push(target, 0xf8, mem);
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

fn(void, machine_jmpn0, i64 where, bytecode target, indexes replace_pointers) {
    // RBX contains the actual jump location, whilst RCX contains the address right after the JMP.

    var rbx_place = target->size + 2;
    machine_rbx(0, target, mem);

    var rcx_place = target->size + 2;
    machine_rcx(0, target, mem);

    // cmp rax, 0
    push(target, 0x48, mem);
    push(target, 0x83, mem);
    push(target, 0xf8, mem);
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

fn(void, machine_mmap, bytecode target) {
    // addr(null) size(size) prot(0b11) flags(0x22) fd(-1) offset(0)
    // syscall number
    // machine_eax(9, target, mem);
    machine_eax(9, target, mem);
    // address
    // machine_rdi(0, target, mem);
    machine_zero_rdi(target, mem);
    // len
    // machine_esi(size, target, mem);
    // prot
    machine_edx(0b111, target, mem);
    // flags
    machine_r10d(0x22, target, mem);
    // fd
    machine_r8d(-1, target, mem);
    // offset
    machine_r9d(0, target, mem);
}

fn(void, machine_rax_in_rdi, bytecode target) {
    push(target, 0x48, mem);
    push(target, 0x89, mem);
    push(target, 0xc7, mem);
}

fn(void, machine_munmap, bytecode target) {
    machine_rax(11, target, mem);
    // machine_esi(size, target, mem);
}

// fn(void, machine_mmap, i64 size, bytecode target) {
//     var x = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);

//     // addr(null) size(size) prot(0b11) flags(0x22) fd(-1) offset(0)
//     // syscall number
//     machine_rax(192, target, mem);
//     // address
//     machine_rdi(0, target, mem);
//     // len
//     machine_rsi(min(size, 4096), target, mem);
//     // prot
//     machine_rdx(PROT_READ | PROT_WRITE | PROT_EXEC, target, mem);
//     // flags
//     machine_r10(MAP_ANON | MAP_PRIVATE, target, mem);
//     // fd
//     machine_r8(-1, target, mem);
//     // offset
//     machine_r9(0, target, mem);

//     machine_syscall(target, mem);
// }

fn(void, machine_push_rax, bytecode target) { push(target, 0x50, mem); }
fn(void, machine_pop_rax, bytecode target) { push(target, 0x58, mem); }

// Steps:
// 1. [x] Figure out ELF
// 2. [ ] Figure out DATA segment
// 3. [ ] Figure out TEXT segment
// 4. [ ] Rewrite the arena allocator in assembly, then hand-assemmble it to x86
// 5. [ ] Utilize that to perform calls and returns
// 6. [ ] Rewrite every standard function in the language in assembly then hand-assemble it to
// x86 This process is REQUIRED for addrI, addrD, jmp0, jmpn0, call, ret, set, pop and push

// RET
fn(void, machine_ret, bytecode target) { push(target, 0xc3, mem); }

// Function to initialize and return an ELF header
Elf64_Ehdr create_elf_header() {
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

fn(void, machine_elf, bytecode target) {
    // Generate ELF header
    var  _elf_header                      = create_elf_header();
    char elf_header[ sizeof(Elf64_Ehdr) ] = { 0 };
    memcpy(elf_header, &_elf_header, sizeof(Elf64_Ehdr));

    for (i32 i = 0; i < sizeof(Elf64_Ehdr); i++) {
        var ch = elf_header[ i ];
        push(target, ch, mem);
    }
}

void print_bytecode(i64 size, byte *target) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", target[ i ]);
        if (i % 8 == 7 && i > 0) printf("\n");
    }
    printf("\n");
}

mc get_exec(byte *target, i64 func_start, i64 size) {
    var executable
        = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(executable, target, size);

    return &executable[ func_start ];
}

i64 get_rax_value() {
    i64 rax_value;

    __asm__("mov %%rax, %0" : "=r"(rax_value) : : "%rax");

    return rax_value;
}

i64 get_rdi_value() {
    i64 rdi_value;

    __asm__("mov %%rdi, %0" : "=r"(rdi_value) : : "%rax");

    return rdi_value;
}

i64 get_rdx_value() {
    i64 rdx_value;

    __asm__("mov %%rdx, %0" : "=r"(rdx_value) : : "%rax");

    return rdx_value;
}

fn(void, insert_ptr_space, IR ir, bytecode target) {
    for (i32 i = 0; i < ir.compiler_data->reserves->size; i++) { extend((void *) target, 8, mem); }
}

void replace_bytecode_pointers(
    byte *target, i64 function_offset, i64 st_st_target, indexes pointers) {
    var destination = &target[ -function_offset ];
    for (i64 i = 0; i < pointers->size; i++) {
        var addr = (i64 *) (&destination[ pointers->array[ i ].put_in ]);
        // printf("replacing %li with %lX\n", pointers->array[ i ].index, (i64) destination +
        // pointers->array[ i ].offset);
        *addr = (i64) destination + pointers->array[ i ].value + st_st_target;
    }

    // printf("\n");
}

void test_scribe(IR ir) {
    ground();

    var test             = (bytecode) new (char, 0);
    var replace_pointers = (indexes) new (struct Pointer, 0);

    // Used for separating preprocessing values
    var st_st_target = test->size;
    insert_ptr_space(ir, test, scratch);

    var func_start = test->size;

    {
        // put rax in pointer, put 32 in rax and restore rax from pointer

        // machine_rax_in_ptr(0x0, test, (void *) replace_pointers, st_st_target, scratch);
        // machine_rax(32, test, scratch);
        // machine_ptr_in_rax(0x0, test, (void *) replace_pointers, st_st_target, scratch);
    }

    {
        // mmap, put ptr in memory, unmap it, put it back into rax and return

        machine_mmap(test, scratch);
        machine_esi(32, test, scratch);
        machine_syscall(test, scratch);

        // machine_push_rax(test, scratch);
        machine_rax_in_ptr(0x0, test, replace_pointers, scratch);

        machine_rsi(32, test, scratch);
        machine_ptr_in_rdi(0x0, test, replace_pointers, scratch);
        // machine_pop_rax(test, scratch);
        // machine_rax_in_rdi(test, scratch);
        machine_munmap(test, scratch);

        // Uncomment to unmap the allocated memory and therefore causing a segfault when
        // `*rax_value = 2;` is ran.
        machine_syscall(test, scratch);
        machine_ptr_in_rdi(0x0, test, replace_pointers, scratch);
    }

    {
        // while true loop, hopefully...

        // machine_jmp0(test->size, test, replace_pointers, scratch);
    }

    machine_ret(test, scratch);

    var function_pointer = get_exec(test->array, func_start, test->size);

    replace_bytecode_pointers(
        (byte *) function_pointer, func_start, st_st_target, replace_pointers);

    /*
    // -- Uncomment these to write the x86 output to a file allowing for easier debugging -- 
    var fd = fopen("test.bin", "w");
    fwrite(function_pointer, test->size - func_start, 1, fd);
    fclose(fd);
    */

    // print_bytecode(test->size, test->array);
    // printf("\n");

    print_bytecode(test->size, test->array);
    printf("\n");

    function_pointer();
    int *rax_value = (int *) get_rax_value();
    int *rdi_value = (int *) get_rdi_value();

    print_bytecode(test->size, &((byte *) function_pointer)[ -func_start ]);

    printf("function signature: %p - %li\n", function_pointer, (i64) rdi_value);

    printf("\nRAX: %p\n", rdi_value);
    printf("\n" red("ATTENTION! A SIGSEGV IS EXPECTED BEHAVIOR:") "\n");

    *rdi_value = 2;

    printf("[RAX]: %i\n", *rdi_value);

    release();
}