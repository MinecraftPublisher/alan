# Proposed syntax:
# let ptr = mmap 32;

set area_size 4096;

set main_area 0;
set initialized 0;
set filled 5;

fn num alloc_experiment [
    arg num size;

    dryback main_area;
];

set output [ alloc_experiment 100 ];
tmp filled;