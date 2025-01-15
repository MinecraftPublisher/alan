// // Proposed syntax:
// // let ptr = mmap 32;

// fn void memcpy [
//     arg num src;
//     arg num dst;
//     arg num size;

//     set i 0;

//     while [ sub size i ] [
//         set src_ptr [ add src i ];
//         set dst_ptr [ add dst i ];

//         setp dst_ptr [ getp src_ptr ];

//         set i [ add i 1 ];
//     ];
// ];

// fn num mremap [
//     arg num src;
//     arg num size;

//     set new [ mmap size ];
//     memcpy src new size;
//     munmap src size;

//     dryback new;
// ];

// set area_size 4096;

// // arena_list memory section contains the list of arenas
// // each arena's first element is its number of blocks, then the data for each block.
// // a block has a size, a free value and an address, so 3 elements per block in the 1d array.
// set arena_list_ptr 0;
// set arena_list_size 0;

// fn num alloc_experiment [
//     arg num _size;
//     set size [ add _size 1 ]; // first element is the array length. 

//     set is_not_init [ not arena_list_ptr ];

//     if is_not_init [
//         set arena_list_size [ add 1 3 ];
//         set arena_list_ptr [ mmap arena_list_size ];
//     ];

//     dryback size;
// ];

// set output [ alloc_experiment 100 ];
// tmp output;

set i 1;
set j 2;

fn num test [
    set y 4;

    if [ not 0 ] [
        set z 5;
        set j 7;
        tmp z;
    ];

    dryback y;
];

set out [ test ];
tmp out;

// tmp [ add 55 i ];