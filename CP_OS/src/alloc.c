#include "alloc.h"
#include "freelist_alloc.h"
#include "buddy_alloc.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>

static size_t power_of_two(size_t size) {
    size_t power = 1;
    while (power < size) {
        power *= 2;
    }
    return power;
}

Allocator* createMemoryAllocator(AllocatorType type, size_t memory_size) {
    Allocator *allocator = malloc(sizeof(Allocator));
    if (!allocator) return NULL;

    size_t real_size = memory_size;

    if (type == ALLOC_BUDDY) {
        real_size = power_of_two(memory_size);
    }

    void *memory = mmap(NULL, real_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        free(allocator);
        return NULL;
    }

    allocator->type = type;
    allocator->total_size = real_size;
    allocator->used_size = 0;

    if (type == ALLOC_FREE_LIST) {  
        allocator->impl = freelist_create(memory, real_size);
    } else {
        allocator->impl = buddy_create(memory, real_size);
    }

    return allocator;
}

void* allocator_alloc(Allocator *allocator, size_t size) {  
    if (allocator->type == ALLOC_FREE_LIST) {
        return freelist_alloc(allocator, size);
    } else {
        return buddy_alloc(allocator, size);
    }
}

void allocator_free(Allocator *allocator, void *ptr) {
    if (allocator->type == ALLOC_FREE_LIST) {
        freelist_free(allocator, ptr);
    } else {
        buddy_free(allocator, ptr);
    }
}

void allocator_destroy(Allocator *allocator) {
    if (!allocator) {
        return;
    }

    void *memory;
    if (allocator->type == ALLOC_FREE_LIST) {
        FreeListAllocator *alloc = allocator->impl;
        memory = alloc->memory;
        free(alloc);
    } else {
        BuddyAllocator *alloc = allocator->impl;
        memory = alloc->memory;
        free(alloc);
    }

    munmap(memory, allocator->total_size);
    free(allocator);
}