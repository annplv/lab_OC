#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contract.h"

int main(void) {
    printf("Current lib: A\n");
    printf("1 A B - PrimeCount(A,B)\n");
    printf("2 A B - GCF(A,B)\n");
    char cmd;
    while (scanf(" %c", &cmd) == 1) {
        if (cmd == 'q'){
            break;
        } else if (cmd == '1') {
            int A, B;
            if (scanf("%d %d", &A, &B) != 2){
              break;
            }
            int res = PrimeCount(A, B);
            printf("PrimeCount(%d,%d) = %d\n", A, B, res);
        } else if (cmd == '2') {
            int A, B;
            if (scanf("%d %d", &A, &B) != 2){ 
              break;
            }
            int res = GCF(A, B);
            printf("GCF(%d,%d) = %d\n", A, B, res);
        } else {
            printf("Unknown cmd\n");
        }
    }
    return 0;
}
