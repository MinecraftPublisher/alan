# Proposed syntax:
# let ptr = mmap 32;

# arena_list memory section contains the list of arenas

set area_size 4096;

set arena_list_ptr 0;
set arena_list_size 0;
set arena_list_cur 0;

fn void memcpy [
    arg num src;
    arg num dest;
    arg num size;
    set i 0;
    while i [
        tmp 1;
    ];
];

fn num alloc_experiment [
    arg num size2;
    set size [ add size2 1 ]; # first element is the array length.

    # TODO
];

set output [ alloc_experiment 100 ];
tmp output;