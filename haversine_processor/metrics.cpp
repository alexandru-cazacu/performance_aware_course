#include <intrin.h> // __rdtsc()
#include <windows.h> // QueryPerformanceFrequency(), ...

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