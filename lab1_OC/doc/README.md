gcc main.c -o main
gcc child1.c -o child1
gcc child2.c -o child2

./main

strace ./main
