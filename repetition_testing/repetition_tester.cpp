enum TestMode : u32 {
    TestMode_Uninitialized,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

struct RepetitionTestResults {
    u64 testCount;
    u64 totalTime;
    u64 minTime;
    u64 maxTime;
};

struct RepetitionTester {
    u64 targetProceddedByteCount;
    u64 cpuTimerFreq;
    u64 tryForTime;
    u64 testsStartedAt;
    
    TestMode mode;
    
    bool printNewMinimums;
    u32 openBlockCount;
    u32 closeBlockCount;
    u64 timeAccumulatedOnThisTest;
    u64 bytesAccumulatedOnThisTest;
    
    RepetitionTestResults results;
};

static double seconds_from_cpu_time(double cpuTime, u64 cpuTimerFreq) {
    double result = 0.0;
    
    if (cpuTimerFreq) {
        result = (cpuTime / (double)cpuTimerFreq);
    }
    
    return result;
}

static void tester_print_time(const char* label, double cpuTime, u64 cpuTimerFreq, u64 byteCount) {
    printf("%s: %.0f", label, cpuTime);
    
    if (cpuTimerFreq) {
        double seconds = seconds_from_cpu_time(cpuTime, cpuTimerFreq);
        printf(" (%fms)", 1000.0f * seconds);
        
        if (byteCount) {
            double gigabyte = (1024.0f * 1024.0f * 1024.0f);
            double bestBandwidth = byteCount / (gigabyte * seconds);
            printf(" %fgb/s", bestBandwidth);
        }
    }
}

static void print_results(RepetitionTestResults results, u64 cpuTimerFreq, u64 byteCount) {
    tester_print_time("Min", (double)results.minTime, cpuTimerFreq, byteCount);
    printf("\n");
    
    tester_print_time("Max", (double)results.maxTime, cpuTimerFreq, byteCount);
    printf("\n");
    
    if (results.testCount) {
        tester_print_time("Avg", (double)results.totalTime / (double)results.testCount, cpuTimerFreq, byteCount);
        printf("\n");
    }
}

static void tester_print_time(const char* label, u64 cpuTime, u64 cpuTimerFreq, u64 byteCount) {
    tester_print_time(label, (double)cpuTime, cpuTimerFreq, byteCount);
}

static void tester_error(RepetitionTester* tester, const char* message) {
    tester->mode = TestMode_Error;
    fprintf(stderr, "ERROR: %s\n", message);
}

static void new_test_wave(RepetitionTester* tester, u64 targetProceddedByteCount, u64 cpuTimerFreq, u32 secondsToTry = 10) {
    if (tester->mode == TestMode_Uninitialized) {
        tester->mode = TestMode_Testing;
        tester->targetProceddedByteCount = targetProceddedByteCount;
        tester->cpuTimerFreq = cpuTimerFreq;
        tester->printNewMinimums = true;
        tester->results.minTime = (u64)-1;
    } else if (tester->mode == TestMode_Completed) {
        tester->mode = TestMode_Testing;
        
        if (tester->targetProceddedByteCount != targetProceddedByteCount) {
            tester_error(tester, "targetProceddedByteCount changed");
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
    tester->timeAccumulatedOnThisTest -= read_cpu_timer();
}

static void tester_end_time(RepetitionTester* tester) {
    tester->closeBlockCount++;
    tester->timeAccumulatedOnThisTest += read_cpu_timer();
}

static void count_bytes(RepetitionTester* tester, u64 byteCount) {
    tester->bytesAccumulatedOnThisTest += byteCount;
}

static bool tester_is_testing(RepetitionTester* tester) {
    if (tester->mode == TestMode_Testing) {
        u64 currentTime = read_cpu_timer();
        
        if (tester->openBlockCount) {
            if (tester->openBlockCount != tester->closeBlockCount) {
                tester_error(tester, "Unbalanded begin_time/end_time");
            }
            
            if (tester->bytesAccumulatedOnThisTest != tester->targetProceddedByteCount) {
                tester_error(tester, "Procedded byte count mismatch");
            }
            
            if (tester->mode == TestMode_Testing) {
                RepetitionTestResults* results = &tester->results;
                u64 elapsedTime = tester->timeAccumulatedOnThisTest;
                results->testCount += 1;
                results->totalTime += elapsedTime;
                
                if (results->maxTime < elapsedTime) {
                    results->maxTime = elapsedTime;
                }
                
                if (results->minTime > elapsedTime) {
                    results->minTime = elapsedTime;
                    
                    // Reset clock to the full trial time
                    tester->testsStartedAt = currentTime;
                    
                    if (tester->printNewMinimums) {
                        tester_print_time("Min", results->minTime, tester->cpuTimerFreq, tester->bytesAccumulatedOnThisTest);
                        printf("               \r");
                    }
                }
                
                tester->openBlockCount = 0;
                tester->closeBlockCount = 0;
                tester->timeAccumulatedOnThisTest = 0;
                tester->bytesAccumulatedOnThisTest = 0;
            }
        }
        
        if ((currentTime - tester->testsStartedAt) > tester->tryForTime) {
            tester->mode = TestMode_Completed;
            
            printf("                                                          \r");
            print_results(tester->results, tester->cpuTimerFreq, tester->targetProceddedByteCount);
        }
    }
    
    bool result = (tester->mode == TestMode_Testing);
    return result;
}