num thread_id [
    spawn [
        puts "Hello World!\n";

        while 1 [
            set news [ fetch ];

            puts news;
        ]
    ]
];

send thread_id "Hi";