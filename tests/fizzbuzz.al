fn num fizzbuzz [
    arg num count;
    num i 1;

    while [ sub count i ] [
        num mod_3 [ mod i 3 ];
        num mod_5 [ mod i 5 ];
        num and_val [ and mod_3 mod_5 ];

        if and_val [ log i ];
        unless and_val [
            unless mod_3 [ puts "Fizz" ];
            unless mod_5 [ puts "Buzz" ];
        ];
        puts "\n";

        set i [ add i 1 ];
    ];
];

fizzbuzz 100;