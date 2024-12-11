# Alan - Array Language
This is a simple compiler for a [memory-safe] programming language where everything (even numbers) are arrays. It is very primitive, and reading the `template.c` file gives you a way better understanding of the built-in functions.

It *may* be turing-complete (I haven't checked, but it probably is), but it requires most commonly used utilities to be written using C interfacing, or written directly into the C templating. An updating including a proper guide and an stdlib alongside troubleshooting coming soon.

# Build:
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
fn say_hello [
    arg name;

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

## 5. Direct C interface
```
fn mod [
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

## 6. Fizzbuzz!
```
fn fizzbuzz [
    arg count;
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

# Updates
- Update 1.p1: Added literal unrolling and caching, optimized function calls and reduced dereference count. The compiled executable now runs about ***100x*** faster!

# TODO and chores
Coming soon in future updates!
- [x] Rewrite parser in C
- [x] Rewrite codegen in C
- [x] Optimize performance
- [x] Eliminate / shorten code
- [ ] Implement direct linux x86 output (maybe?)
- [ ] Bundle code + x86 together to allow runtime code inspection and modification (maybe?)
- [ ] Better error checking (soon)