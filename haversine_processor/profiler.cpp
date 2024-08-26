struct ProfileAnchor {
    u64 tscElapsedExclusive; // Does NOT include children
    u64 tscElapsedInclusive; // DOES include children
    u64 hitCount;
    const char* label;
};

struct Profiler {
    ProfileAnchor Anchors[1024];
    
    u64 startTsc;
    u64 endTsc;
};

static Profiler gProfiler;
static u32 gProfilerParent;

struct ProfileBlock {
    ProfileBlock(const char* label_, u32 anchorIndex_) {
        parentIndex = gProfilerParent;
        
        anchorIndex = anchorIndex_;
        label = label_;
        
        ProfileAnchor* anchor = gProfiler.Anchors + anchorIndex;
        oldTscElapsedInclusive = anchor->tscElapsedInclusive;
        
        gProfilerParent = anchorIndex;
        startTsc = read_cpu_timer();
    }
    
    ~ProfileBlock() {
        u64 elapsed = read_cpu_timer() - startTsc;
        gProfilerParent = parentIndex;
        
        ProfileAnchor* parent = &gProfiler.Anchors[parentIndex];
        ProfileAnchor* anchor = &gProfiler.Anchors[anchorIndex];
        
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

static void begin_profile() {
    gProfiler.startTsc = read_cpu_timer();
}

static void end_profile_and_print() {
    gProfiler.endTsc = read_cpu_timer();
    u64 cpuFreq = estimate_cpu_timer_freq();
    u64 totalCpuElapsed = gProfiler.endTsc - gProfiler.startTsc;
    
    if (cpuFreq) {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (double)totalCpuElapsed / (double)cpuFreq, cpuFreq);
    }
    
    for (int i = 0; i < ARRAY_COUNT(gProfiler.Anchors); i++) {
        ProfileAnchor* anchor = &gProfiler.Anchors[i];
        if (anchor->tscElapsedInclusive) {
            print_elapsed_time(totalCpuElapsed, anchor);
        }
    }
}

#define NAME_CONCAT2(A, B) A##B
#define NAME_CONCAT(A, B) NAME_CONCAT2(A, B)

#define PROFILE_SCOPE(name) ProfileBlock NAME_CONCAT(block, __LINE__)(name, __COUNTER__ + 1)
#define PROFILE_FUNC() PROFILE_SCOPE(__func__)
