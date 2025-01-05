# Alan - Array Language
This is a simple compiler for a [memory-safe] programming language where everything (even numbers) are arrays. It is very primitive, but it is turing complete.

**Note**: I really can't provide an online demo or a screen recording for this. However, it should be fairly easy to clone and compile this with minimal effort. The demo image below is the closest I could get to a real demo (below the image is the explanation):

![Demo image.](https://iili.io/2U8iCnR.jpg)
>Figure A out of A: Top left shows input alan code. The rest of the left side is the disassembled machine code of the program. Top right is the intermediate representation of the program, and bottom right is the compiled machine code output of the program.

## C version
This is a work-in-progress compiler (with a self-estimation of around 90% completion), which __does not use any external dependencies__. Not even an assembler, I wrote the machine code output functions by hand, all by myself. The C compiler uses no external libraries but the standard libc. It takes alan code, parses it, optimizes it, error checks it, converts it into semi-assembly code using IR techniques, then it converts the IR code into x86 machine code that can be ran in C like this:
```c
// ...
mc function_pointer = __x86_64_linux_get_exec(target->array, environment->func_start, target->size);
// ...
function_pointer();
```
with no errors. That's right, the compiled program creates a function for you to run in your code. This means I can even integrate this compiler into my future projects and use it just like you would use an interpreted language, with the difference of the code being actually compiled.

## Fun facts!
- **The code that the compiler produces is actually less code than writing the same program in C. Yes, you heard that right!**
- The name wasn't originally supposed to be a reference to Alan Turing, but later on I realized the correlation.
- The static analyzer is called the Inspector. Why? I don't know, I didn't want to just call it "Static Analyzer". That would be boring. The parser is called Librarian, the intermediate representation generator is called Tourist, and the executable generators are called Scribes.

## Updates
- Update 1.p3C: Static analyzer is now improved, and the compiler can (sloppily) target x86-64 direct bytecode output for linux, but only as JIT.
- Update 1.p2C: Created and improved the C parser + static analyzer. The end goal is compiling programs directly to x86_64 machine code as an executable.
- Update 1.p1: Added literal unrolling and caching, optimized function calls and reduced dereference count. The compiled executable now runs about ***100x*** faster!

---

Bad news (yes I hid this over here):
- The C version is not fully usable yet (however it does produce direct x86 output!)
- The typescript version is outdated and it requires most commonly used utilities to be written using C interfacing, or written directly into the C templating. An updating including a proper guide and an stdlib alongside troubleshooting coming soon.

---

## Experimental Alcc (C rewrite) build:
>This version is in-development and is not fully fledged, as in it only produces a Just-In-Time compiled function pointer for 64-bit x86 systems running linux. Even that isn't fully fledged out.
1. Install clang and make
2. Clone the repo and cd into it
3. Run `make`. The parser executable will be in the `out/` folder and it will also automatically run it with the test file of the day.

## Build:
>Currently the typescript version that emits C code and compiles it using `clang` is available. The C version is trying to emit machine code directly with no dependency requirements and it is a work in progress.
1. Install bun
2. Clone the repo and cd into the repo and then to the `ts_version/` folder
3. Run `make build`
4. Compiler executable is now in `ts_version/out/alc`. Run like this: `alc <input_file> <output_executable>`

**Note**: The compiler requires `clang` in path to work.

# Some examples:

## 1. Hello World
```
puts "Hello World!\n";
```

## 2. Variables
```
list x;
set x [push x 'H'];
set x [push x 'i'];

puts x;
```

## 3. Functions
```
fn void say_hello [
    arg list name;

    puts "Hello, ";
    puts name;
    puts "!\n";
]

say_hello "Alan";
```

## 4. Loops
```
num i 0;
while [ sub 256 i ] [ # Print all ASCII characters
    puts i;
    set i [ add i 1 ];
];
```

## 5. Direct C interface (Deprecated in C version)
```
fn num mod [
    arg a;
    arg b;

    c{ $single($value(sym_a) %  $value(sym_b), &cur_scope) };
];

log [ mod 5 2 ];
```

>**Warning**: REALLY not recommended unless you know what you're doing. If you do:
>1. Your C interface MUST return an object of type `A`.
>2. All variables are prefixed with `sym_`.
>3. All functions are prefixed with `fn_`.
>4. This feature is ONLY supported in the Typescript version of the compiler.

## 6. Fizzbuzz!
```
fn void fizzbuzz [
    arg num count;
    num i 1;

    while [ sub count i ] [
        num mod_3 [ mod i 3 ];
        num mod_5 [ mod i 5 ];
        num and_val [ and mod_3 mod_5 ];

        if and_val [ log i; puts "\n" ];
        unless and_val [
            unless mod_3 [ puts "Fizz" ];
            unless mod_5 [ puts "Buzz" ];
            puts "\n";
        ];

        set i [ add i 1 ];
    ];
];

fizzbuzz 100;
```

## 7. Add two strings together
```
fn list cat [
    arg list left;
    arg list right;

    list result;

    set left_len [ len left ];
    set right_len [ len right ];

    set li 0;
    set ri 0;

    while [ sub left_len li ] [
        push result [ get left li ];
        set li [ dec li ];
    ];

    while [ sub right_len ri ] [
        push result [ get right ri ];
        set ri [ dec ri ];
    ];

    ret result;
];

print [ cat "Hello, " "World!" ];
```
This should print `Hello, World!`.


# C version
The C version is memory-safe, does all the parsing on its own and currently can convert the AST into sloppy JIT bytecode. The currently supported IR language is very basic and has only 12 instructions. It is semi-stack-based, pushing arguments to a dynamic stack for when functions are called. The #1 planned architecture to be supported is x86, #2 being javascript, #3 being C, #4 being arm, #5 being RISC and #6 being my own custom cpu architecture, [bit](https://github.com/MinecraftPublisher/bit).

- The following code snippet:

```
num i [ add 2 5 ];
log i;

puts "Hello World!";
```

- Converts into this IR (the x86 output for complex functions isn't done yet):

```
main:
    CONST 5
    PUSH
    CONST 2
    PUSH
    CALL __0x28                ; add
    SET 0x0_1                  ; i
    ADDR [0x0_1]               ; [i]
    PUSH
    CALL __0x26                ; log
    ADDR 0x0_1F                ; "Hello World!"
    PUSH
    CALL __0x24                ; puts
```

# TODO and chores
Coming soon in future updates! I've started work on the x86 output, so expect this spot to be pretty cluttered.
- [ ] Implement an arena stack
- [ ] `call(complicated...);`
- [ ] `ret(complicated...);`
- [ ] Better syntax?
- [ ] Write a better stdlib (trying)
- [ ] Implement carrying for arithmetic functions (eg. addc)
- [ ] Bundle code + x86 together to allow runtime code inspection and modification (maybe?)
- [ ] Write code elimination
- [ ] Add bytecode optimizations
- [ ] Write the allocator in alan itself
- [x] `pop(mov [cur_stack], %rex);`
- [x] `push(push_stack %rex);`
- [x] `jmp0(cmp %rex; jmp0 [addr]);`
- [x] `jmpn0(reverse of jmp0)`
- [x] `addrI(mov %rex, addr);`
- [x] `addrD(mov %rex, [addr]);`
- [x] `set(mov [addr], %rex);`
- [x] Implement a call stack
- [x] Implement direct linux x86 output (almost done)
- [x] Better error checking (soon) (kinda done?)
- [x] Eliminate / shorten code (Code structuralized)
- [x] Rewrite parser in C
- [x] Rewrite codegen in C
- [x] Optimize performance
- [x] Finish the IR emitter (done besides bug fixes and testing)
- [x] Eliminate / shorten code pass 1

