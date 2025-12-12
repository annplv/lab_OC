#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_STR 256

static int is_vowel(char c) {
    return (c=='a'||c=='e'||c=='i'||c=='o'||c=='u'||
            c=='A'||c=='E'||c=='I'||c=='O'||c=='U');
}

static void remove_vowels(char *s) {
    char *src = s, *dst = s;
    while (*src) {
        if (!is_vowel(*src))
            *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

static void mmap_append(const char *fname, const char *data, size_t len) {
    int fd = open(fname, O_RDWR | O_CREAT, 0666);
    if (fd == -1) return;

    struct stat st;
    fstat(fd, &st);

    off_t old_size = st.st_size;
    off_t new_size = old_size + len;
    ftruncate(fd, new_size);

    long page = sysconf(_SC_PAGESIZE);
    off_t offset = old_size & ~(page - 1);
    size_t map_len = (old_size - offset) + len;

    char *map = mmap(NULL, map_len, PROT_WRITE, MAP_SHARED, fd, offset);
    if (map != MAP_FAILED) {
        memcpy(map + (old_size - offset), data, len);
        msync(map, map_len, MS_SYNC);
        munmap(map, map_len);
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);
    char buffer[MAX_STR];

    while (fgets(buffer, MAX_STR, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        remove_vowels(buffer);
        strcat(buffer, "\n");
        mmap_append(argv[1], buffer, strlen(buffer));
    }
    return 0;
}

