CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c11

all: bitmask_pool_allocator

bitmask_pool_allocator: main.c
	$(CC) $(CFLAGS) main.c -o bitmask_pool_allocator

clean:
	rm -f bitmask_pool_allocator
