num i 3;

if [ sub i 3 ] [ puts "Hello world!" ];

# always prints hello world
fn test1 [
    num x 1;
    log x;
];

fn test2 [
    list x;
    test1;
    log x;
];

test2;
# problem: here _i_ becomes 1 because the symbols wouldn't change.
# solution: maintain a symbol cache and revert it after every call?
# problem: 