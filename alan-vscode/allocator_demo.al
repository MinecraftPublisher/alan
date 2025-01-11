# Proposed syntax:
# let ptr = mmap 32;


#set area_size 4096;

# arena_list memory section contains the list of arenas
# each arena's first element is its number of blocks, then 

set arena_list_ptr 0;
set arena_list_size 0;
set arena_list_cur 0;

fn void memcpy [
    arg num src;
    arg num dst;
    arg num size;

    set i 0;

    while [ sub size i ] [
        set src_ptr [ add src i ];
        set dst_ptr [ add dst i ];

        setp dst_ptr [ getp src_ptr ];

        set i [ add i 1 ];
    ];
];

fn num alloc_experiment [
    arg num _size;
    set size [ add _size 1 ]; # first element is the array length.

];

set output [ alloc_experiment 100 ];
tmp output;