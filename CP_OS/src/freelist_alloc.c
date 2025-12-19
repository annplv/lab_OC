#include "freelist_alloc.h"
#include <stdlib.h>

FreeListAllocator* freelist_create(void *memory, size_t size) {
    FreeListAllocator *alloc = malloc(sizeof(FreeListAllocator));
    alloc->memory = memory;
    alloc->size = size;
    alloc->free_list = (FreeBlock*)memory;  
    alloc->free_list->size = size - sizeof(FreeBlock);
    alloc->free_list->next = NULL;
    return alloc; 
}

void* freelist_alloc(Allocator *base, size_t size) {  
    FreeListAllocator *alloc = base->impl;
    FreeBlock *best = NULL;
    FreeBlock *best_prev = NULL;
    FreeBlock *prev = NULL;
    FreeBlock *curr = alloc->free_list;

    while (curr) {
        if (curr->size >= size) {
            if (!best || curr->size < best->size) {
                best = curr;
                best_prev = prev;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    if (!best) {
        return NULL;
    }

    if (best->size <= size + sizeof(FreeBlock)) {  
        if (best_prev) {
            best_prev->next = best->next;
        } else {
            alloc->free_list = best->next;
        }
        base->used_size += best->size;
        return (void*)(best + 1);
    }

    FreeBlock *next = (FreeBlock*)((char*)(best + 1) + size);  
    next->size = best->size - size - sizeof(FreeBlock);
    next->next = best->next;

    if (best_prev) {
        best_prev->next = next;
    } else {
        alloc->free_list = next;
    }

    best->size = size;
    base->used_size += size;
    return (void*)(best + 1);
}

void freelist_free(Allocator *base, void *ptr) {
    if (!ptr) {
        return;
    }
    FreeListAllocator *alloc = base->impl;
    FreeBlock *block = ((FreeBlock*)ptr) - 1;
    base->used_size -= block->size;  

    // вставка в список
    FreeBlock **curr = &alloc->free_list;
    while (*curr && *curr < block){
        curr = &(*curr)->next;
    }
    block->next = *curr;
    *curr = block;  

    // слияние с соседями
    if (block->next && (char*)(block + 1) + block->size == (char*)block->next) {  
        block->size += sizeof(FreeBlock) + block->next->size;
        block->next = block->next->next;
    }
    if (curr != &alloc->free_list) {
        FreeBlock *prev = alloc->free_list;
        while (prev && prev->next != block) {
            prev = prev->next;
        }

        if (prev && (char*)(prev + 1) + prev->size == (char*)block) {
            prev->size += sizeof(FreeBlock) + block->size;
            prev->next = block->next;
        }
    }
}