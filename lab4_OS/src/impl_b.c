#include "contract.h"
#include <stdlib.h>
#include <string.h>

//Решето Эратосфена 
int PrimeCount(int A, int B) {
    if (A > B) {
        int t = A; A = B; B = t;
    }
    if (B < 2) return 0;
    if (A < 2) A = 2;

    int n = B;
    char *isPrime = (char*)malloc(n + 1);
    if (!isPrime) return 0;
    memset(isPrime, 1, n + 1);
    isPrime[0] = isPrime[1] = 0;

    for (int p = 2; p * (long long)p <= n; ++p) {
        if (isPrime[p]) {
            for (int q = p * p; q <= n; q += p)
                isPrime[q] = 0;
        }
    }
    int cnt = 0;
    for (int i = A; i <= B; ++i){ 
      if (isPrime[i]) ++cnt;
    }
    free(isPrime);
    return cnt;
}

//наивный (перебор) 
int GCF(int A, int B) {
    if (A < 0) A = -A;
    if (B < 0) B = -B;
    if (A == 0) return B;
    if (B == 0) return A;
    int m = (A < B) ? A : B;
    for (int d = m; d >= 1; --d) {
        if (A % d == 0 && B % d == 0) return d;
    }
    return 1;
}
