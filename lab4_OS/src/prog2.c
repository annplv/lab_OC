#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

typedef int (*fnPrimeCount)(int,int);
typedef int (*fnGCF)(int,int);

int main(void) {
    const char *libA = "./libimplA.so";
    const char *libB = "./libimplB.so";

    void *handle = NULL;
    fnPrimeCount prime = NULL;
    fnGCF gcf = NULL;

    handle = dlopen(libA, RTLD_NOW);
    if (!handle) {
        return 1;
    }
    prime = (fnPrimeCount)dlsym(handle, "PrimeCount");
    gcf   = (fnGCF)dlsym(handle, "GCF");
    if (!prime || !gcf) {
        dlclose(handle);
        return 1;
    }
    printf("Current lib: A\n");
    printf("0 - switch lib\n");
    printf("1 A B - PrimeCount(A,B)\n");
    printf("2 A B - GCF(A,B)\n");

    int cur = 0; // 0 - A, 1 - B
    char cmd;
    while (scanf(" %c", &cmd) == 1) {
        if (cmd == 'q'){
            break;
        } else if (cmd == '0') {
            dlclose(handle);
            cur = !cur;
            const char *lib = (cur == 0) ? libA : libB;
            handle = dlopen(lib, RTLD_NOW);
            if (!handle) {
                return 1;
            }
            prime = (fnPrimeCount)dlsym(handle, "PrimeCount");
            gcf = (fnGCF)dlsym(handle, "GCF");
            if (!prime || !gcf) {
                dlclose(handle);
                return 1;
            }
            printf("Switched to %s\n", lib);
        } else if (cmd == '1') {
            int A, B;
            if (scanf("%d %d", &A, &B) != 2){
              break;
            }
            printf("PrimeCount(%d,%d) = %d\n", A, B, prime(A,B));
        } else if (cmd == '2') {
            int A, B;
            if (scanf("%d %d", &A, &B) != 2){
              break;
            }
            printf("GCF(%d,%d) = %d\n", A, B, gcf(A,B));
        } else {
            printf("Unknown cmd\n");
        }
    }
    if (handle){
      dlclose(handle);
    }
    return 0;
}