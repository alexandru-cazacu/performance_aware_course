#ifndef PROFILER
#define PROFILER 0
#endif

#ifndef PROFILER_BLOCK_TIMER
#define PROFILER_BLOCK_TIMER read_cpu_timer
#endif

#if PROFILER

struct ProfileAnchor {
    u64 tscElapsedExclusive; // Does NOT include children
    u64 tscElapsedInclusive; // DOES include children
    u64 hitCount;
    const char* label;
};

static ProfileAnchor gProfileAnchors[4096];
static u32 gProfilerParent;

struct ProfileBlock {
    ProfileBlock(const char* label_, u32 anchorIndex_) {
        parentIndex = gProfilerParent;
        
        anchorIndex = anchorIndex_;
        label = label_;
        
        ProfileAnchor* anchor = gProfileAnchors + anchorIndex;
        oldTscElapsedInclusive = anchor->tscElapsedInclusive;
        
        gProfilerParent = anchorIndex;
        startTsc = PROFILER_BLOCK_TIMER();
    }
    
    ~ProfileBlock() {
        u64 elapsed = PROFILER_BLOCK_TIMER() - startTsc;
        gProfilerParent = parentIndex;
        
        ProfileAnchor* parent = &gProfileAnchors[parentIndex];
        ProfileAnchor* anchor = &gProfileAnchors[anchorIndex];
        
        parent->tscElapsedExclusive -= elapsed;
        anchor->tscElapsedExclusive += elapsed;
        anchor->tscElapsedInclusive = oldTscElapsedInclusive + elapsed;
        anchor->hitCount++;
        anchor->label = label;
    }
    
    const char* label;
    u64 oldTscElapsedInclusive;
    u64 startTsc;
    u32 parentIndex;
    u32 anchorIndex;
};

static void print_elapsed_time(u64 totalTscElapsed, ProfileAnchor* anchor) {
    double percent = 100.0 * ((double)anchor->tscElapsedExclusive / (double)totalTscElapsed);
    printf("  %s[%llu]: %llu (%.2f%%", anchor->label, anchor->hitCount, anchor->tscElapsedExclusive, percent);
    
    if (anchor->tscElapsedInclusive != anchor->tscElapsedExclusive) {
        double percentWithChildren = 100.0 * ((double)anchor->tscElapsedInclusive / (double)totalTscElapsed);
        printf(", %.2f%% w/children", percentWithChildren);
    }
    
    printf(")\n");
}

static void print_anchor_data(u64 totalCpuElapsed) {
    for (int i = 0; i < ARRAY_COUNT(gProfileAnchors); i++) {
        ProfileAnchor* anchor = &gProfileAnchors[i];
        if (anchor->tscElapsedInclusive) {
            print_elapsed_time(totalCpuElapsed, anchor);
        }
    }
}

#define NAME_CONCAT2(A, B) A##B
#define NAME_CONCAT(A, B) NAME_CONCAT2(A, B)

#define PROFILE_SCOPE(name) ProfileBlock NAME_CONCAT(block, __LINE__)(name, __COUNTER__ + 1)
#define PROFILE_FUNC() PROFILE_SCOPE(__func__)

#define PROFILER_ASSERT static_assert(__COUNTER__ < ARRAY_COUNT(gProfileAnchors), "Number of profile points exceeds size of Profiler::Anchors")

#else // PROFILER

#define PROFILE_SCOPE(name)
#define PROFILE_FUNC()
#define print_anchor_data(...)

#define PROFILER_ASSERT

#endif // PROFILER

struct Profiler {
    u64 startTsc;
    u64 endTsc;
};

static Profiler gProfiler;

static u64 estimate_block_timer_freq() {
    u64 millisToWait = 100;
    u64 osFreq = get_os_timer_freq();
    
    u64 blockStart = PROFILER_BLOCK_TIMER();
    u64 osStart = read_os_timer();
    u64 osEnd = 0;
    u64 osElapsed = 0;
    u64 osWaitTime = osFreq * millisToWait / 1000;
    
    while (osElapsed < osWaitTime) {
        osEnd = read_os_timer();
        osElapsed = osEnd - osStart;
    }
    
    u64 blockEnd = PROFILER_BLOCK_TIMER();
    u64 blockElapsed = blockEnd - blockStart;
    u64 blockFreq = 0;
    
    if (osElapsed) {
        blockFreq = osFreq * blockElapsed / osElapsed;
    }
    
    return blockFreq;
}

static void begin_profile() {
    gProfiler.startTsc = PROFILER_BLOCK_TIMER();
}

static void end_profile_and_print() {
    gProfiler.endTsc = PROFILER_BLOCK_TIMER();
    u64 cpuFreq = estimate_block_timer_freq();
    u64 totalCpuElapsed = gProfiler.endTsc - gProfiler.startTsc;
    
    if (cpuFreq) {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (double)totalCpuElapsed / (double)cpuFreq, cpuFreq);
    }
    
    print_anchor_data(totalCpuElapsed);
}