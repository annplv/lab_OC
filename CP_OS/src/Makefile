all:
	gcc -std=c11 -Wall -Wextra -O2 \
	    alloc.c freelist_alloc.c buddy_alloc.c test_alloc.c \
	    -o alloc

clean:
	rm -f alloc
