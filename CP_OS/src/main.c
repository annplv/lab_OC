#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>


//Аллокатор со списком свободных блоков
typedef struct FreeBlock {  
    size_t size;   
    struct FreeBlock *next;  
} FreeBlock;

typedef struct {
    void *memory;  
    size_t size;   
    FreeBlock *free_list;   
} FreeListAllocator;

static FreeListAllocator* freelist_create(void *memory, size_t size) {
    FreeListAllocator *alloc = malloc(sizeof(FreeListAllocator));
    alloc->memory = memory;
    alloc->size = size;
    alloc->free_list = (FreeBlock*)memory;  
    alloc->free_list->size = size - sizeof(FreeBlock);
    alloc->free_list->next = NULL;
    return alloc; 
}

static void* freelist_alloc(Allocator *base, size_t size) {  
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

static void freelist_free(Allocator *base, void *ptr) {
    if (!ptr) {
        return;
    }
    FreeListAllocator *alloc = base->impl;
    FreeBlock *block = ((FreeBlock*)ptr) - 1;
    base->used_size -= block->size;  

    // вставка в список с упорядочиванием по адресу
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


//Аллокатор двойников
#define MAX_ORDER 16

typedef struct BuddyBlock {
    struct BuddyBlock *next;
} BuddyBlock;

typedef struct {
    void *memory;
    int max_order;
    BuddyBlock *free_lists[MAX_ORDER + 1];
} BuddyAllocator;

static int order_for_size(size_t size) {
    int order = 0;
    size_t block = 1;
    while (block < size) {
        block <<= 1;
        order++;
    }
    return order;
}

static BuddyAllocator* buddy_create(void *memory, size_t size) {
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

static void* buddy_alloc(Allocator *base, size_t size) {
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

static void buddy_free(Allocator *base, void *ptr) {
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


static size_t power_of_two(size_t size) {
    size_t power = 1;
    while (power < size) {
        power *= 2;
    }
    return power;
}


//AllocatorCreate
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


//Единые функции
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


//Тестирование
#define MEMORY_SIZE 65536
#define ITERATIONS 1000
#define BLOCK_SIZE 32

int main() {
    Allocator *fl = createMemoryAllocator(ALLOC_FREE_LIST, MEMORY_SIZE);
    Allocator *bd = createMemoryAllocator(ALLOC_BUDDY, MEMORY_SIZE);

    void *ptrs[ITERATIONS];
    int count;
    clock_t start, end;

    //Free List alloc 
    count = 0;
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        ptrs[i] = allocator_alloc(fl, BLOCK_SIZE);
        if (!ptrs[i]) {
            break;
        }
        count++;
    }
    end = clock();

    printf("FreeList alloc time: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("FreeList usage: %.2f%%\n", 100.0 * fl->used_size / fl->total_size);

    //Free List free 
    start = clock();
    for (int i = 0; i < count; i++) {
        allocator_free(fl, ptrs[i]);
    }
    end = clock();

    printf("FreeList free time: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);

    //Buddy alloc 
    count = 0;
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        ptrs[i] = allocator_alloc(bd, BLOCK_SIZE);
        if (!ptrs[i]) {
            break;
        }
        count++;
    }
    end = clock();

    printf("Buddy alloc time: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Buddy usage: %.2f%%\n", 100.0 * bd->used_size / bd->total_size);

    //Buddy free 
    start = clock();
    for (int i = 0; i < count; i++) {
        allocator_free(bd, ptrs[i]);
    }
    end = clock();

    printf("Buddy free time: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);

    allocator_destroy(fl);
    allocator_destroy(bd);
    return 0;
}



