# Memory layout:
# First item will be the 'initialized? value
# Second item will be the block count
# From then on it will be the addresses to different blocks

# type allocate_storage = Array[Array[Block]]
# type Block = [  ]

num alloc_data 0;

fn num unsafe_allocate [
    # Should return a pointer
    
    arg num size;

    set pointer [ mmap size ];

    dryback pointer;
];

fn void unsafe_free [
    arg num ptr;
    munmap ptr;
];

set alloc_data [ unsafe_allocate 32 ];
unsafe_free alloc_data;