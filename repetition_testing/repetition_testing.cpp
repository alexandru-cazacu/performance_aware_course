#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h> // _stat64

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#include "../haversine_processor/string.cpp"
#include "../haversine_processor/metrics.cpp"
#include "repetition_tester.cpp"
#include "read_overhead_test.cpp"

struct TestFunction {
    const char* name;
    read_overhead_test_func* func;
};

TestFunction gTestFunctions[] = {
    { "fread", read_via_fread },
    { "_read", read_via_read },
    { "ReadFile", read_via_read_file },
};

int main(int argc, char** argv) {
    u64 cpuTimerFreq = estimate_cpu_timer_freq();
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [existing filename]\n", argv[0]);
        return 0;
    }
    
    char* fileName = argv[1];
    
    struct __stat64 stat;
    _stat64(fileName, &stat);
    
    ReadParams params = {};
    params.dest = allocate_string(stat.st_size);
    params.fileName = fileName;
    
    if (params.dest.count <= 0) {
        fprintf(stderr, "ERROR: Test data size must be non-zero\n");
        return 0;
    }
    
    RepetitionTester testers[ARRAY_COUNT(gTestFunctions)] = {};
    
    while (true) {
        for (int i = 0; i < ARRAY_COUNT(gTestFunctions); i++) {
            RepetitionTester* tester = testers + i;
            TestFunction testFunc = gTestFunctions[i];
            
            printf("\n--- %s ---\n", testFunc.name);
            new_test_wave(tester, params.dest.count, cpuTimerFreq);
            testFunc.func(tester, &params);
        }
    }
    
    return 0;
}