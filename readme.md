# Bitmask Pool Allocator Benchmark

A high-performance, single-cycle $O(1)$ lock-free memory pool allocator written in C11. 
This project features a custom memory allocator tracked via an atomic 64-bit mask,

benchmarked against standard `malloc`/`free` using a lightning-fast lockless Xorshift32 pseudo-random engine based on George marsaglia.

## How to Run
```bash
make
./bitmask_memory_allocator

## Benchmark Output
This is benchmark of bitmask pool allocator against malloc 

Here is the execution profile running 10,000,000 allocation cycles on a Fedora Linux environment:

Same random generator seed: 0xDEADBEEF for both bitmask allocator and malloc 
Custom Bitmask Pool:  0.016718 seconds
Standard malloc/free: 0.097412 seconds

Custom pool is 5.83x FASTER than malloc() under true fragmentation.
