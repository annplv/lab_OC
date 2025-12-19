#include "alloc.h"
#include <stdio.h>
#include <time.h>

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



