#define MemoryBarrier asm volatile("mfence" ::: "memory")
