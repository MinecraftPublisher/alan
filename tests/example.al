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
        set li [ inc li ];
    ];

    while [ sub right_len ri ] [
        push result [ get right ri ];
        set ri [ inc ri ];
    ];

    ret result;
];

print [ cat "Hello, " "World!" ];