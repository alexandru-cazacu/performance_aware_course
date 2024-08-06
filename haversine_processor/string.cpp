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

static String allocate_string(size_t count) {
    String result = {};
    result.data = (u8*)malloc(count);
    
    if (result.data) {
        result.count = count;
    } else {
        fprintf(stderr, "ERROR: Unable to allocate %llu bytes.\n", count);
    }
    
    return result;
}

static void free_string(String* string) {
    if (string->data) {
        free(string->data);
    }
    
    *string = {};
}