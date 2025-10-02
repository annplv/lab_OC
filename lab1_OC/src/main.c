#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_STR 256

int main() {
    int pipe1[2], pipe2[2];
    char file1[128], file2[128];

    // создаём два канала
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(1);
    }

    printf("Введите имя файла для child1: ");
    scanf("%127s", file1);
    printf("Введите имя файла для child2: ");
    scanf("%127s", file2);
    getchar(); // убрать \n после scanf

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        // child1
        dup2(pipe1[0], STDIN_FILENO);  // читаем из pipe1
        close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        execl("./child1", "child1", file1, NULL);
        perror("execl child1");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        // child2
        dup2(pipe2[0], STDIN_FILENO);  // читаем из pipe2
        close(pipe2[1]);
        close(pipe1[0]); close(pipe1[1]);
        execl("./child2", "child2", file2, NULL);
        perror("execl child2");
        exit(1);
    }

    // Родительский процесс
    close(pipe1[0]);
    close(pipe2[0]);

    char buffer[MAX_STR];
    printf("Введите строки (exit для завершения):\n");
    while (1) {
        if (fgets(buffer, MAX_STR, stdin) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0; // убрать \n

        if (strcmp(buffer, "exit") == 0) break;

        size_t len = strlen(buffer);
        strcat(buffer, "\n"); // добавляем перевод строки для детей

        if (len <= 10) {
            write(pipe1[1], buffer, strlen(buffer));
        } else {
            write(pipe2[1], buffer, strlen(buffer));
        }
    }

    close(pipe1[1]);
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);

    printf("Родитель завершил работу.\n");
    return 0;
}


