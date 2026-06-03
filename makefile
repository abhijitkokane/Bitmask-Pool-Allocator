CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c11

all: bitmask_memory_allocator

bitmask_memory_allocator: main.c
	$(CC) $(CFLAGS) main.c -o bitmask_memory_allocator

clean:
	rm -f bitmask_memory_allocator
