fn list cat [
    arg list left;
    arg list right;

    list result;

    set left_len [ len left ];
    set right_len [ len right ];

    set li 0;
    set ri 0;

    while [ sub left_len li ] [
        set result [ push result [ get left li ] ];
        set li [ add li 1 ];
    ];
];

cat "Hi" "Hello";