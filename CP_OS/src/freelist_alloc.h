#ifndef FREELIST_ALLOC_H
#define FREELIST_ALLOC_H

#include <stddef.h>
#include "alloc.h"

typedef struct FreeBlock {  
    size_t size;   
    struct FreeBlock *next;  
} FreeBlock;

typedef struct {
    void *memory;  
    size_t size;   
    FreeBlock *free_list;   
} FreeListAllocator;

FreeListAllocator* freelist_create(void *memory, size_t size);
void* freelist_alloc(Allocator *base, size_t size);
void freelist_free(Allocator *base, void *ptr);

#endif