struct String {
    u64 count;
    u8* data;
};

#define CONSTANT_STRING(str) { sizeof(str) - 1, (u8 *)(str) }

static bool is_in_bound(String source, u64 at) {
    bool result = at < source.count;
    return result;
}

static bool are_equal(String a, String b) {
    if (a.count != b.count) {
        return false;
    }
    
    for (u64 i = 0; i < a.count; i++) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    
    return true;
}