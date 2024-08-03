enum JsonTokenType {
    
};

struct JsonToken {
    
};

struct JsonElement {
    
};

struct JsonParser {
    String source;
};

static bool is_json_digit() {
    
}

static bool is_json_whitespace() {
    
}

static bool is_parsing() {
    
}

static void parser_error() {
    
}

static void parse_keyword() {
    
}

static JsonToken get_json_token() {
    
}

static JsonElement* parse_json_element() {
    
}

static JsonElement* parse_json_list() {
    
}

static JsonElement* parse_json(String inputJson) {
    JsonParser parser = {};
    parser.source = inputJson;
    
    JsonElement* result;
    //JsonElement* result = parse_json_element(&parser, {}, get_json_token(&parser));
    
    return result;
}

static void free_json() {
    
}

static JsonElement* lookup_element() {
    
}

static double convert_json_sign() {
    
}

static double convert_json_number() {
    
}

static double convert_element_to_double() {
    
}

static u64 parse_haversine_pairs(String inputJson, u64 maxPairCount, HaversinePair* oPairs) {
    u64 pairCount = 0;
    
    JsonElement* json = parse_json(inputJson);
    //JsonElement* pairsArray = lookup_element(json, CONSTANT_STRINT("pairs"));
    
    // TODO(alex): Extract pairs
    
    // TODO(alex): Remove. To shut up compiler warning.
    (oPairs);
    (maxPairCount);
    
    return pairCount;
}