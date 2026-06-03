# Bitmask Memory Allocator Benchmark

A high-performance, single-cycle $O(1)$ lock-free memory pool allocator written in C11. 
This project features a custom memory allocator tracked via an atomic 64-bit mask,

benchmarked against standard `malloc`/`free` using a lightning-fast lockless Xorshift32 pseudo-random engine based on George marsaglia.

## How to Run
```bash
make
./bitmask_memory_allocator
