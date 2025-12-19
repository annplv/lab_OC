#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>  

typedef enum {  
    ALLOC_FREE_LIST,
    ALLOC_BUDDY
} AllocatorType;  

typedef struct Allocator Allocator;  

struct Allocator {
    AllocatorType type; 
    void *impl;            
    size_t total_size;   
    size_t used_size;    
};

Allocator* createMemoryAllocator(AllocatorType type, size_t memory_size);  

void* allocator_alloc(Allocator *allocator, size_t size);  
void allocator_free(Allocator *allocator, void *ptr);
void allocator_destroy(Allocator *allocator);

#endif 
