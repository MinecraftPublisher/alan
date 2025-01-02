# Alan - Array Language
This is a simple compiler for a [memory-safe] programming language where everything (even numbers) are arrays. It is very primitive, and reading the `template.c` file gives you a way better understanding of the built-in functions.

It *may* be turing-complete (I haven't checked, but it probably is), but it requires most commonly used utilities to be written using C interfacing, or written directly into the C templating. An updating including a proper guide and an stdlib alongside troubleshooting coming soon.

## Updates
- Update 1.p2C: Created and improved the C parser + static analyzer. The end goal is compiling programs directly to x86_64 machine code as an executable.
- Update 1.p1: Added literal unrolling and caching, optimized function calls and reduced dereference count. The compiled executable now runs about ***100x*** faster!

## Fun facts
- The name wasn't originally supposed to be a reference to Alan Turing, but later on I realized the correlation.
- The static analyzer is called the Overlord. Why? I don't know, I didn't want to just call it "Static Analyzer". That would be boring.

---

## Experimental Alcc (C rewrite) build:
>This version is in-development and is not fully fledged, as in it only produces an intermediate representation and not an output x86 binary.
1. Install clang and make
2. Clone the repo and cd into it
3. Run `make`. The parser executable will be in the `out/` folder and it will also automatically run it with a fizzbuzz test file.

## Build:
>Currently only the typescript version that emits C code and compiles it using `clang` is available. The C version will emit machine code directly with no dependency requirements and it is a work in progress.
1. Install bun
2. Clone the repo and cd into the repo and then to the `ts_version/` folder
3. Run `make build`
4. Compiler executable is now in `./out/alc`. Run like this: `alc <input_file> <output_executable>`

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

## 5. Direct C interface (Deprecated in barebones)
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
The C version is memory-safe, does all the parsing on its own and currently can convert the AST into an Intermediate Representation. The currently supported IR language is very basic and has only 10 instructions. It is semi-stack-based, pushing arguments to a dynamic stack for when functions are called. I have plans to write plugins that would convert this IR code into machine code directly, and this means I'll be able to support new architectures without having to recycle and rewrite huge parts of the code. The #1 planned architecture to be supported is x86, with #2 being arm, #3 being RISC and #4 being my own custom cpu architecture, [bit](https://github.com/MinecraftPublisher/bit).

- The following code snippet:

```
num i [ add 2 5 ];
log i;

puts "Hello World!";
```

- Converts into this IR:

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
- [ ] Implement a call stack and an arena stack (for return calls)
- [ ] `pop(mov [cur_stack], %rex);`
- [ ] `push(push_stack %rex);`
- [ ] `call(complicated...);`
- [ ] `ret(complicated...);`
- [ ] `jmp0(cmp %rex; jmp0 [addr]);`
- [ ] `jmpn0(reverse of jmp0)`
- [ ] `addrI(mov %rex, addr);`
- [ ] `addrD(mov %rex, [addr]);`
- [ ] `set(mov [addr], %rex);`
- [ ] Implement direct linux x86 output (working on it)
- [ ] Better error checking (soon) (kinda done?)
- [ ] Better syntax?
- [ ] Write a better stdlib
- [ ] Implement carrying for arithmetic functions (eg. addc)
- [ ] Eliminate / shorten code
- [ ] Bundle code + x86 together to allow runtime code inspection and modification (maybe?)
- [x] Rewrite parser in C
- [x] Rewrite codegen in C
- [x] Optimize performance
- [x] Finish the IR emitter (done besides bug fixes and testing)
- [x] Eliminate / shorten code pass 1