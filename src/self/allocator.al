# Proposed syntax:
# let ptr = mmap 32;

set area_size 4096;

set main_area 0;
set initialized 0;
set filled 0;

fn num alloc_experiment [
    arg num size;

    # mmap memory
    if [ not initialized ] [
        set main_area [ mmap area_size ];
        set initialized 25;
    ];

    set ptr [ add main_area filled ];
    set filled [ add filled size ];

    dryback main_area;
];

alloc_experiment 100;
tmp main_area;