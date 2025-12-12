CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -fPIC -O2
LDFLAGS = -ldl -lm

all: libimplA.so libimplB.so prog1 prog2

libimplA.so: impl_a.c contract.h
	$(CC) $(CFLAGS) -shared impl_a.c -o libimplA.so -lm

libimplB.so: impl_b.c contract.h
	$(CC) $(CFLAGS) -shared impl_b.c -o libimplB.so -lm

prog1: prog1.c contract.h libimplA.so
	$(CC) prog1.c -L. -limplA -Wl,-rpath=. -o prog1 -lm

prog2: prog2.c contract.h libimplA.so libimplB.so
	$(CC) prog2.c -o prog2 $(LDFLAGS)

clean:
	rm -f *.o *.so prog1 prog2
