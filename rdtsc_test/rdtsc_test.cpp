#include <stdio.h>
#include <stdint.h>
#include <intrin.h> // __rdtsc()

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

inline u64 read_cpu_timer() {
    return __rdtsc();
}

int main() {
    int result = 1;
    
    u64 value = read_cpu_timer();
    
    fprintf(stdout, "rdtsc: %llu\n", value);
    
    return result;
}