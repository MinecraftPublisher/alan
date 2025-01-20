#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

//

#include "macros.h"
#include "types.h"

void arena_free(Arena *arena) {
    adeb(printf("Free(%p)\n", arena));
    adeb(dump_stack());
    if(!arena) return;
    if (arena->initialized) {
        for (i32 i = 0; i < arena->blocks->size; i++) {
            adeb(printf("Block.Free(%p)\n", arena->blocks->array[ i ].data));
            munmap(arena->blocks->array[ i ].data, arena->blocks->array[ i ].size);
        }
        munmap(arena->blocks->array, arena->blocks->size * arena->blocks->unit);
        munmap(arena->blocks, sizeof(A(Block)));
        arena->initialized = 0;
        arena->blocks      = NULL;
    }
}

#define halign(x) printf("\e[%iG",x)

#define alloc(arena, size) __alloc(arena, size, (char*)__FUNCTION__, __FILE__, __LINE__)
void *__alloc(Arena *arena, i32 size, char* function, char *filename, int line) {
    adeb(printf(red("Location: %s() %s line %i    "), function, filename, line));
    adeb(halign(64));
    
    if (!arena->initialized) {
        arena->initialized = 1;
        arena->blocks      = mmap(
            NULL, sizeof(A(Block)), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (arena->blocks == NULL) {
            fprintf(stderr, "Memory allocation failed in alloc!\n");
            exit(EXIT_FAILURE);
        }
        arena->blocks->size  = 0;
        arena->blocks->unit  = sizeof(Block);
        arena->blocks->array = NULL;
    }

    for (i32 i = 0; i < arena->blocks->size; i++) {
        Block *block = &arena->blocks->array[ i ];
        if (block->size - block->used >= size) {
            void *ptr = &block->data[ block->used ];
            block->used += size;
            adeb(printf(green("Alloc(%p, %i, %p)") "\n", arena, size, ptr));
            return ptr;
        }
    }

    Block new_block;
    new_block.size = max(size * 2, ARENA_SIZE_UNIT);

    new_block.used = size;
    new_block.data
        = mmap(null, new_block.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_block.data == NULL) {
        fprintf(stderr, "Memory allocation failed in alloc!\n");
        exit(EXIT_FAILURE);
    }
    memset(new_block.data, 0, new_block.size); // bad for performance?

    (arena->blocks->size)++;
    var previous         = arena->blocks->array;
    arena->blocks->array = mmap(
        NULL,
        sizeof(Block) * arena->blocks->size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    // arena->blocks->array = realloc(arena->blocks->array, sizeof(Block) * arena->blocks->size);
    if (arena->blocks->array == NULL) {
        fprintf(stderr, "Memory allocation failed in alloc (realloc)!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(arena->blocks->array, previous, sizeof(Block) * (arena->blocks->size - 1));
    munmap(previous, sizeof(Block) * (arena->blocks->size - 1));

    arena->blocks->array[ arena->blocks->size - 1 ] = new_block;

    adeb(printf(yellow("Block.New(%p, %i, %p)") "\n", arena, size, new_block.data));
    return new_block.data;
}