
struct ProfileAnchor {
    
};

struct Profiler {
    ProfileAnchor Anchors[1024];
};

static Profiler gProfiler;

struct ProfileBlock {
    ProfileBlock(const char* label_, u32 anchorIndex_) {
        label = label_;
        anchorIndex = anchorIndex_;
        startTsc = read_cpu_timer();
    }
    
    ~ProfileBlock() {
        
    }
    
    const char* label;
    u32 anchorIndex;
    u64 startTsc;
};

static void print_elapsed_time() {
    
}

static void begin_profile() {
    
}

static void end_profile_and_print() {
    
}

#define NAME_CONTACT2(A, B) A##B
#define NAME_CONTACT(A, B) NAME_CONTACT2(A, B)

#define PROFILE_SCOPE(name) ProfileBlock NAME_CONTACT(block, __LINE__)(name, __COUNTER__ + 1)
#define PROFILE_FUNC() PROFILE_SCOPE(__func__)
