#include "contract.h"
#include <math.h>
#include <stdlib.h>

//наивный (проверка простоты делением) 
static int is_prime_naive(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    int r = (int)sqrt((double)n);
    for (int d = 3; d <= r; d += 2)
        if (n % d == 0) return 0;
    return 1;
}

int PrimeCount(int A, int B) {
    if (A > B) {
        int t = A; A = B; B = t;
    }
    if (B < 2) return 0;
    if (A < 2) A = 2;
    int cnt = 0;
    for (int i = A; i <= B; ++i)
        if (is_prime_naive(i)) ++cnt;
    return cnt;
}

//алгоритм Евклида 
int GCF(int A, int B) {
    if (A < 0) A = -A;
    if (B < 0) B = -B;
    if (A == 0) return B;
    if (B == 0) return A;
    while (B != 0) {
        int t = A % B;
        A = B;
        B = t;
    }
    return A;
}
