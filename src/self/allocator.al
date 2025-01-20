// // // Proposed syntax:
// // let ptr = mmap 32;

// // TODO: Replace variables if they're only used once

// fn void memcpy [
//     arg num src;
//     arg num dst;
//     arg num size;

//     set i 0;

//     while [ sub size i ] [
//         // dst[i] = src[i]
//         setp [ add dst i ] [ getp [ add src i ] ];
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

// // block[3]: [ int size, int cur, int* ptr ]
// // arena[1+n]:: [ int count, block* list ]
// // arena_list[1+n]: [ int count, arena* list ]

// set check 9;

// fn num arena_alloc [
//     arg num _size;
//     set size _size;

//     // initialize the arena list
//     if [ not arena_list_ptr ] [
//         set arena_list_size 2; // elements are split into groups of two, 
//         // irst item of those is the arena's length and the next is the block list pointer.
//         set arena_list_ptr [ mmap arena_list_size ];

//         // it would be wayyy better if i didn't allocate any arenas when starting...
//         setp arena_list_ptr 1; // allocate one arena with one block in it for starters.
//         set arena_ptr [ add arena_list_ptr 8 ];
//         setp arena_ptr 1;
//         set block_ptr [ add arena_ptr 8 ]; // also block_size_ptr
//         set block_cur_ptr [ add block_ptr 8 ];
//         setp [ add block_ptr 16 ] [ mmap area_size ];

//         setp block_ptr area_size;
//         setp block_cur_ptr 0;
//     ];

//     dryback size;
// ];

// tmp [ arena_alloc 100 ];

fn num test1 [
    set x 1;

    if 1 [ set x 2 ];
    if 0 [ set x 3 ];

    dryback x;
];

tmp [ test1 ];