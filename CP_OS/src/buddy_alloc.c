#include "buddy_alloc.h"
#include <stdlib.h>
#include <stdint.h>

int order_for_size(size_t size) {
    int order = 0;
    size_t block = 1;
    while (block < size) {
        block <<= 1;
        order++;
    }
    return order;
}

BuddyAllocator* buddy_create(void *memory, size_t size) {
    BuddyAllocator *alloc = malloc(sizeof(BuddyAllocator));
    alloc->memory = memory;
    alloc->max_order = order_for_size(size);
    for (int i = 0; i <= MAX_ORDER; i++) {
        alloc->free_lists[i] = NULL;
    }
    alloc->free_lists[alloc->max_order] = (BuddyBlock*)memory;
    alloc->free_lists[alloc->max_order]->next = NULL;
    return alloc;
}

void* buddy_alloc(Allocator *base, size_t size) {
    BuddyAllocator *alloc = base->impl;
    int order = order_for_size(size + sizeof(int));
    int current = order;

    while (current <= alloc->max_order && alloc->free_lists[current] == NULL) {
        current++;
    }

    if (current > alloc->max_order) {
        return NULL;
    }

    BuddyBlock *block = alloc->free_lists[current];
    alloc->free_lists[current] = block->next;

    while (current > order) {
        current--;
        BuddyBlock *buddy = (BuddyBlock*)((char*)block + (1 << current));
        buddy->next = alloc->free_lists[current];
        alloc->free_lists[current] = buddy;
    }

    int *header = (int*)block;
    *header = order;
    base->used_size += (1 << order);
    return (void*)(header + 1);
}

void buddy_free(Allocator *base, void *ptr) {
    if (!ptr) {
        return;
    }
    BuddyAllocator *alloc = base->impl;
    int *header = ((int*)ptr) - 1;
    int order = *header;
    BuddyBlock *block = (BuddyBlock*)header;
    base->used_size -= (1 << order);

    while (order < alloc->max_order) {
        uintptr_t offset = (uintptr_t)block - (uintptr_t)alloc->memory;
        uintptr_t buddy_offset = offset ^ (1 << order);
        BuddyBlock *buddy = (BuddyBlock*)((uintptr_t)alloc->memory + buddy_offset);

        BuddyBlock **curr = &alloc->free_lists[order];
        while (*curr && *curr != buddy) {
            curr = &(*curr)->next;
        }
        if (*curr != buddy) {
            break;
        }

        *curr = buddy->next;
        if (buddy < block) {
            block = buddy;
        }
        order++;
    }
    block->next = alloc->free_lists[order];
    alloc->free_lists[order] = block;
}