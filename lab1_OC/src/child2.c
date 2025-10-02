#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 256

// удаляем гласные
void remove_vowels(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (strchr("aeiouyAEIOUY", *src) == NULL) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Не передано имя файла\n");
        exit(1);
    }

    FILE *f = fopen(argv[1], "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    char buffer[MAX_STR];
    while (fgets(buffer, MAX_STR, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        remove_vowels(buffer);
        fprintf(f, "%s\n", buffer);
    }

    fclose(f);
    return 0;
}

