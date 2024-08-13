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

static u64 estimate_cpu_timer_freq() {
    u64 millisToWait = 100;
    u64 osFreq = get_os_timer_freq();
    
    u64 cpuStart = read_cpu_timer();
    u64 osStart = read_os_timer();
    u64 osEnd = 0;
    u64 osElapsed = 0;
    u64 osWaitTime = osFreq * millisToWait / 1000;
    
    while (osElapsed < osWaitTime) {
        osEnd = read_os_timer();
        osElapsed = osEnd - osStart;
    }
    
    u64 cpuEnd = read_cpu_timer();
    u64 cpuElapsed = cpuEnd - cpuStart;
    u64 cpuFreq = 0;
    
    if (osElapsed) {
        cpuFreq = osFreq * cpuElapsed / osElapsed;
    }
    
    return cpuFreq;
}

int main(int argc, char** argv) {
    u64 millisToWait = 1000;
    
    if (argc == 2) {
        millisToWait = atol(argv[1]);
    }
    
    int result = 1;
    
    u64 osFreq = get_os_timer_freq();
    
    u64 cpuStart = read_cpu_timer();
    u64 osStart = read_os_timer();
    u64 osEnd = 0;
    u64 osElapsed = 0;
    u64 osWaitTime = osFreq * millisToWait / 1000;
    
    while (osElapsed < osWaitTime) {
        osEnd = read_os_timer();
        osElapsed = osEnd - osStart;
    }
    
    u64 cpuEnd = read_cpu_timer();
    u64 cpuElapsed = cpuEnd - cpuStart;
    u64 cpuFreq = 0;
    
    if (osElapsed) {
        cpuFreq = osFreq * cpuElapsed / osElapsed;
    }
    
    printf("   OS Freq: %llu (reported)\n", osFreq);
    printf("  OS Timer: %llu -> %llu = %llu elapsed\n", osStart, osEnd, osElapsed);
    printf("OS Seconds: %.4f\n", (double)osElapsed / (double)osFreq);
    printf("CPU Timer: %llu -> %llu = %llu elapsed\n", cpuStart, cpuEnd, cpuElapsed);
    printf(" CPU Freq: %llu (guessed)\n", cpuFreq);
    
    return result;
}