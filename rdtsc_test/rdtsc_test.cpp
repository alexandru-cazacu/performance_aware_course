#include <stdio.h>
#include <stdint.h>
#include <intrin.h> // __rdtsc()

#include <windows.h> // QueryPerformanceFrequency(), ...

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

inline u64 read_cpu_timer() {
    return __rdtsc();
}

static u64 get_os_timer_freq() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

static u64 read_os_timer() {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

int main() {
    int result = 1;
    
    u64 cpuTimer = read_cpu_timer();
    u64 osTimer = read_os_timer();
    
    fprintf(stdout, "rdtsc: %llu\n", cpuTimer);
    fprintf(stdout, "OS: %llu\n", osTimer);
    
    return result;
}