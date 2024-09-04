enum TestMode : u32 {
    TestMode_Uninitialized,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

enum RepetitionValueType {
    RepValue_TestCount,
    
    RepValue_CpuTimer,
    RepValue_MemPageFaults,
    RepValue_ByteCount,
    
    RepValue_Count,
};

struct RepetitionValue {
    u64 e[RepValue_Count];
};

struct RepetitionTestResults {
    RepetitionValue total;
    RepetitionValue min;
    RepetitionValue max;
};

struct RepetitionTester {
    u64 targetProcessedByteCount;
    u64 cpuTimerFreq;
    u64 tryForTime;
    u64 testsStartedAt;
    
    TestMode mode;
    
    bool printNewMinimums;
    u32 openBlockCount;
    u32 closeBlockCount;
    
    RepetitionValue accumulatedOnThisTest;
    RepetitionTestResults results;
};

static double seconds_from_cpu_time(double cpuTime, u64 cpuTimerFreq) {
    double result = 0.0;
    
    if (cpuTimerFreq) {
        result = (cpuTime / (double)cpuTimerFreq);
    }
    
    return result;
}

static void tester_print_value(const char* label, RepetitionValue value, u64 cpuTimerFreq) {
    u64 testCount = value.e[RepValue_TestCount];
    double divisor = testCount ? (double)testCount : 1;
    
    double e[RepValue_Count];
    
    for(u32 eIndex = 0; eIndex < ARRAY_COUNT(e); ++eIndex) {
        e[eIndex] = (double)value.e[eIndex] / divisor;
    }
    
    printf("%s: %.0f", label, e[RepValue_CpuTimer]);
    
    if(cpuTimerFreq) {
        double seconds = seconds_from_cpu_time(e[RepValue_CpuTimer], cpuTimerFreq);
        printf(" (%fms)", 1000.0f * seconds);
        
        if(e[RepValue_ByteCount] > 0) {
            double gigabyte = (1024.0f * 1024.0f * 1024.0f);
            double bandwidth = e[RepValue_ByteCount] / (gigabyte * seconds);
            printf(" %fgb/s", bandwidth);
        }
    }
    
    if(e[RepValue_MemPageFaults] > 0) {
        printf(" PF: %0.4f (%0.4fk/fault)",
               e[RepValue_MemPageFaults],
               e[RepValue_ByteCount] / (e[RepValue_MemPageFaults] * 1024.0));
    }
}

static void print_results(RepetitionTestResults results, u64 cpuTimerFreq) {
    tester_print_value("Min", results.min, cpuTimerFreq);
    printf("\n");
    
    tester_print_value("Max", results.max, cpuTimerFreq);
    printf("\n");
    
    tester_print_value("Avg", results.total, cpuTimerFreq);
    printf("\n");
}

static void tester_error(RepetitionTester* tester, const char* message) {
    tester->mode = TestMode_Error;
    fprintf(stderr, "ERROR: %s\n", message);
}

static void new_test_wave(RepetitionTester* tester, u64 targetProcessedByteCount, u64 cpuTimerFreq, u32 secondsToTry = 10) {
    if (tester->mode == TestMode_Uninitialized) {
        tester->mode = TestMode_Testing;
        tester->targetProcessedByteCount = targetProcessedByteCount;
        tester->cpuTimerFreq = cpuTimerFreq;
        tester->printNewMinimums = true;
        tester->results.min.e[RepValue_CpuTimer] = (u64)-1;
    } else if (tester->mode == TestMode_Completed) {
        tester->mode = TestMode_Testing;
        
        if (tester->targetProcessedByteCount != targetProcessedByteCount) {
            tester_error(tester, "targetProcessedByteCount changed");
        }
        
        if (tester->cpuTimerFreq != cpuTimerFreq) {
            tester_error(tester, "CPU frequency changed");
        }
    }
    
    tester->tryForTime = secondsToTry * cpuTimerFreq;
    tester->testsStartedAt = read_cpu_timer();
}

static void tester_begin_time(RepetitionTester* tester) {
    tester->openBlockCount++;
    
    RepetitionValue* accum = &tester->accumulatedOnThisTest;
    accum->e[RepValue_MemPageFaults] -= read_os_page_fault_count();
    accum->e[RepValue_CpuTimer] -= read_cpu_timer();
}

static void tester_end_time(RepetitionTester* tester) {
    RepetitionValue* accum = &tester->accumulatedOnThisTest;
    accum->e[RepValue_CpuTimer] += read_cpu_timer();
    accum->e[RepValue_MemPageFaults] += read_os_page_fault_count();
    
    tester->closeBlockCount++;
}

static void count_bytes(RepetitionTester* tester, u64 byteCount) {
    RepetitionValue* accum = &tester->accumulatedOnThisTest;
    accum->e[RepValue_ByteCount] += byteCount;
}

static bool tester_is_testing(RepetitionTester* tester) {
    if (tester->mode == TestMode_Testing) {
        RepetitionValue accum = tester->accumulatedOnThisTest;
        u64 currentTime = read_cpu_timer();
        
        if (tester->openBlockCount) {
            if (tester->openBlockCount != tester->closeBlockCount) {
                tester_error(tester, "Unbalanded begin_time/end_time");
            }
            
            if (accum.e[RepValue_ByteCount] != tester->targetProcessedByteCount) {
                tester_error(tester, "Processed byte count mismatch");
            }
            
            if (tester->mode == TestMode_Testing) {
                RepetitionTestResults* results = &tester->results;
                
                accum.e[RepValue_TestCount] = 1;
                for(u32 eIndex = 0; eIndex < ARRAY_COUNT(accum.e); ++eIndex)
                {
                    results->total.e[eIndex] += accum.e[eIndex];
                }
                
                if(results->max.e[RepValue_CpuTimer] < accum.e[RepValue_CpuTimer])
                {
                    results->max = accum;
                }
                
                if(results->min.e[RepValue_CpuTimer] > accum.e[RepValue_CpuTimer])
                {
                    results->min = accum;
                    
                    // Whenever we get a new minimum time, we reset the clock to the full trial time
                    tester->testsStartedAt = currentTime;
                    
                    if(tester->printNewMinimums)
                    {
                        tester_print_value("Min", results->min, tester->cpuTimerFreq);
                        printf("                                   \r");
                    }
                }
                
                tester->openBlockCount = 0;
                tester->closeBlockCount = 0;
                tester->accumulatedOnThisTest = {};
            }
        }
        
        if ((currentTime - tester->testsStartedAt) > tester->tryForTime) {
            tester->mode = TestMode_Completed;
            
            printf("                                                          \r");
            print_results(tester->results, tester->cpuTimerFreq);
        }
    }
    
    bool result = (tester->mode == TestMode_Testing);
    return result;
}