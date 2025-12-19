#ifndef BUDDY_ALLOC_H
#define BUDDY_ALLOC_H

#include <stddef.h>
#include "alloc.h"

#define MAX_ORDER 16

typedef struct BuddyBlock {
    struct BuddyBlock *next;
} BuddyBlock;

typedef struct {
    void *memory;
    int max_order;
    BuddyBlock *free_lists[MAX_ORDER + 1];
} BuddyAllocator;

BuddyAllocator* buddy_create(void *memory, size_t size);
void* buddy_alloc(Allocator *base, size_t size);
void buddy_free(Allocator *base, void *ptr);

#endif