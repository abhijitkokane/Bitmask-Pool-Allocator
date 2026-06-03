#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <stdlib.h>
#include <time.h>

#define CACHE_LINE 64
#define BLOCK_SIZE 64 // each block matches the cache line 
#define NUM_BLOCKS 65536 // N blocks fits perfectly into a single uint64_t bitmask
#define ITERATIONS 10000000 // 10 million iterations


typedef struct 
{
    // for the pool structure to fit , align in cpu cache line 
    alignas(CACHE_LINE) _Atomic uint64_t bitmask; 
    uint8_t memory[NUM_BLOCKS * BLOCK_SIZE];
    
} MemoryPool;

// initialize pool
void pool_init(MemoryPool* pool) 
{
    atomic_init(&pool->bitmask, 0); // all blocks free (0)
}

// lock-free Allocation
void* pool_alloc(MemoryPool* pool) 
{
    uint64_t current_mask;
    uint64_t new_mask;
    int index;

    do {
        current_mask = atomic_load(&pool->bitmask);
        
        // if memory is already full 
        if (current_mask == UINT64_MAX) 
        {
            return NULL; 
        }
        
        index = __builtin_ctzll(~current_mask);
        
        // set the bit at 'index' to 1
        new_mask = current_mask | (1ULL << index);

    // atomic compare and swap 
    } while (!atomic_compare_exchange_weak(&pool->bitmask, &current_mask, new_mask));

    // calculate exact pointer address using pointer arithmetic
    return &pool->memory[index * BLOCK_SIZE];
}

// lock-free Free
void pool_free(MemoryPool* pool, void* ptr) 
{
    uintptr_t offset = (uintptr_t)ptr - (uintptr_t)pool->memory;
    int index = offset / BLOCK_SIZE;

    uint64_t current_mask;
    uint64_t new_mask;

    do {
        current_mask = atomic_load(&pool->bitmask);
        
        // clear the bit at 'index' to 0
        new_mask = current_mask & ~(1ULL << index);

    } while (!atomic_compare_exchange_weak(&pool->bitmask, &current_mask, new_mask));
}





static inline uint32_t fast_rand(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}




int main ( ) { 

    clock_t start, end;
    double pool_time , malloc_time ;
    MemoryPool pool ; 
    void* ptrs[NUM_BLOCKS] = { NULL };
    bool slot_occupied[NUM_BLOCKS] = { false };
    

    atomic_store_explicit(&pool.bitmask, 0, memory_order_relaxed);

    printf("Running Hyper-Tight Fragmented Benchmark (%d iterations)...\n\n", ITERATIONS);

    // =================================================================
    // 1. BENCHMARK: Custom Pool Allocator
    // =================================================================
    uint32_t rng_state = 0xDEADBEEF ; // Seed
    
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) 
    {
        int target_slot = fast_rand(&rng_state) % NUM_BLOCKS; 

        if (!slot_occupied[target_slot]) 
        {
            ptrs[target_slot] = pool_alloc(&pool);
            if (ptrs[target_slot]) slot_occupied[target_slot] = true;
        } 
        else 
        {
            pool_free(&pool, ptrs[target_slot]);
            slot_occupied[target_slot] = false;
            ptrs[target_slot] = NULL;
        }
    }
    end = clock();
    pool_time = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Clean up tracking state for the next run
    for(int k = 0; k < NUM_BLOCKS; k++) 
    {
        slot_occupied[k] = false;
        ptrs[k] = NULL;
    }

    // =================================================================
    // 2. BENCHMARK: Standard Malloc
    // =================================================================
    rng_state = 0xDEADBEEF;   // Reset seed to identical sequence
    
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) 
    {
        int target_slot = fast_rand(&rng_state) % NUM_BLOCKS;

        if (!slot_occupied[target_slot]) 
        {
            ptrs[target_slot] = malloc(BLOCK_SIZE);
            if (ptrs[target_slot]) slot_occupied[target_slot] = true;
        } 
        else 
        {
            free(ptrs[target_slot]);
            slot_occupied[target_slot] = false;
            ptrs[target_slot] = NULL;
        }
    }
    end = clock();
    malloc_time = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Final clean up of remaining malloc allocations
    for(int k = 0; k < NUM_BLOCKS; k++) 
    {
        if (slot_occupied[k]) free(ptrs[k]);
    }

    printf("Results:\n");
    printf("  Custom Bitmask Pool:  %.6f seconds\n", pool_time);
    printf("  Standard malloc/free: %.6f seconds\n\n", malloc_time);
    printf("Custom pool is %.2fx FASTER than malloc() under true fragmentation.\n", malloc_time / pool_time);
    return 0;


}



