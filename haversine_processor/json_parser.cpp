enum JsonTokenType {
    Token_end_of_stream,
    Token_error,
    
    Token_open_brace,    // {
    Token_open_bracket,  // [
    Token_close_brace,   // }
    Token_close_bracket, // ]
    Token_comma,         // ,
    Token_colon,         // :
    Token_semi_colon,    // ;
    Token_string_literal,
    Token_number,
    Token_true,
    Token_false,
    Token_null,
    
    Token_count,
};

struct JsonToken {
    JsonTokenType type;
    String value;
};

struct JsonElement {
    String label;
    String value;
    JsonElement* firstSubElement;
    JsonElement* nextSibling;
};

struct JsonParser {
    String source;
    u64 at;
    bool hadError;
};

static bool is_json_digit(String source, u64 at) {
    bool result = false;
    
    if (is_in_bound(source, at)) {
        u8 val = source.data[at];
        result = (val >= '0') && (val <= '9');
    }
    
    return result;
}

static bool is_json_whitespace(String source, u64 at) {
    bool result = false;
    
    if (is_in_bound(source, at)) {
        u8 val = source.data[at];
        result = (val == ' ') || (val == '\t') || (val == '\n') || (val == '\r');
    }
    
    return result;
}

static bool is_parsing(JsonParser* parser) {
    bool result = !parser->hadError && is_in_bound(parser->source, parser->at);
    return result;
}

static void parser_error(JsonParser* parser, JsonToken token, const char* message) {
    parser->hadError = true;
    fprintf(stderr, "ERROR: \"%.*s\" - %s\n", (u32)token.value.count, (char*)token.value.data, message);
}

static void parse_keyword(String source, u64* at, String keywordRemaining, JsonTokenType type, JsonToken* result) {
    if ((source.count - *at) >= keywordRemaining.count) {
        String check = source;
        check.data += *at;
        check.count = keywordRemaining.count;
        
        if (are_equal(check, keywordRemaining)) {
            result->type = type;
            result->value.count += keywordRemaining.count;
            *at += keywordRemaining.count;
        }
    }
}

static JsonToken get_json_token(JsonParser* parser) {
    JsonToken result = {};
    
    String source = parser->source;
    u64 at = parser->at;
    
    while (is_json_whitespace(source, at)) {
        ++at;
    }
    
    if (is_in_bound(source, at)) {
        result.type = Token_error;
        result.value.count = 1;
        result.value.data = source.data + at;
        u8 val = source.data[at++];
        
        switch (val) {
            case '{': { result.type = Token_open_brace; } break;
            case '[': { result.type = Token_open_bracket; } break;
            case '}': { result.type = Token_close_brace; } break;
            case ']': { result.type = Token_close_bracket; } break;
            case ',': { result.type = Token_comma; } break;
            case ':': { result.type = Token_colon; } break;
            case ';': { result.type = Token_semi_colon; } break;
            
            case 'f': {
                parse_keyword(source, &at, CONSTANT_STRING("alse"), Token_false, &result);
            } break;
            
            case 'n': {
                parse_keyword(source, &at, CONSTANT_STRING("ull"), Token_null, &result);
            } break;
            
            case 't': {
                parse_keyword(source, &at, CONSTANT_STRING("rue"), Token_true, &result);
            } break;
            
            case '"': {
                result.type = Token_string_literal;
                
                u64 stringStart = at;
                
                while (is_in_bound(source, at) && (source.data[at] != '"')) {
                    if (is_in_bound(source, (at + 1)) && (source.data[at] == '\\') && (source.data[at + 1] == '"')) {
                        // Skip escaped quotation marks.
                        ++at;
                    }
                    
                    ++at;
                }
                
                result.value.data = source.data + stringStart;
                result.value.count = at - stringStart;
                
                // Skip trailing quotation marks
                if (is_in_bound(source, at)) {
                    ++at;
                }
            } break;
            
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                u64 start = at - 1;
                result.type = Token_number;
                
                // Move past a leading negative sign if one exists
                if ((val == '-') && is_in_bound(source, at)) {
                    val = source.data[at++];
                }
                
                // If the leading digit wasn't 0, parse any digits before the decimal point
                if (val != '0') {
                    while (is_json_digit(source, at)) {
                        ++at;
                    }
                }
                
                // If there is a decimal point, parse any digits after the decimal point
                if (is_in_bound(source, at) && (source.data[at] == '.')) {
                    ++at;
                    
                    while (is_json_digit(source, at)) {
                        ++at;
                    }
                }
                
                // If it's in scientific notation, parse any digits after the "e"
                if (is_in_bound(source, at) && ((source.data[at] == 'e') || (source.data[at] == 'E'))) {
                    ++at;
                    
                    if (is_in_bound(source, at) && ((source.data[at] == '+') || (source.data[at] == '-'))) {
                        ++at;
                    }
                    
                    while (is_json_digit(source, at)) {
                        ++at;
                    }
                }
                
                result.value.count = at - start;
            } break;
            
            default: {
            } break;
        }
    }
    
    parser->at = at;
    
    return result;
}

static JsonElement* parse_json_list(JsonParser* parser, JsonTokenType endType, bool hasLabels);
static JsonElement* parse_json_element(JsonParser* parser, String label, JsonToken value) {
    PROFILE_FUNC();
    
    bool valid = true;
    
    JsonElement* subElement = 0;
    
    if (value.type == Token_open_bracket) {
        subElement = parse_json_list(parser, Token_close_bracket, false);
    } else if (value.type == Token_open_brace) {
        subElement = parse_json_list(parser, Token_close_brace, true);
    } else if((value.type == Token_string_literal) ||
              (value.type == Token_true) ||
              (value.type == Token_false) ||
              (value.type == Token_null) ||
              (value.type == Token_number)) {
        // NOTE(casey): Nothing to do here, since there is no additional data
    } else {
        valid = false;
    }
    
    JsonElement* result = 0;
    
    if (valid) {
        result = (JsonElement*)malloc(sizeof(JsonElement));
        result->label = label;
        result->value = value.value;
        result->firstSubElement = subElement;
        result->nextSibling = 0;
    }
    
    return result;
}

static JsonElement* parse_json_list(JsonParser* parser, JsonTokenType endType, bool hasLabels) {
    JsonElement* firstElement = {};
    JsonElement* lastElement = {};
    
    while (is_parsing(parser)) {
        String label = {};
        JsonToken value = get_json_token(parser);
        
        if (hasLabels) {
            if (value.type == Token_string_literal) {
                label = value.value;
                
                JsonToken colon = get_json_token(parser);
                if (colon.type == Token_colon) {
                    value = get_json_token(parser);
                } else {
                    parser_error(parser, colon, "Expected colon after field name");
                }
            } else if (value.type != endType) {
                parser_error(parser, value, "Unexpected token in JSON");
            }
        }
        
        JsonElement* element = parse_json_element(parser, label, value);
        if (element) {
            lastElement = (lastElement ? lastElement->nextSibling : firstElement) = element;
        } else if (value.type == endType) {
            break;
        } else {
            parser_error(parser, value, "Unexpected token in JSON");
        }
        
        JsonToken comma = get_json_token(parser);
        if (comma.type == endType) {
            break;
        } else if (comma.type != Token_comma) {
            parser_error(parser, comma, "Unexpected token in JSON");
        }
    }
    
    return firstElement;
}

static JsonElement* parse_json(String inputJson) {
    JsonParser parser = {};
    parser.source = inputJson;
    
    JsonElement* result = parse_json_element(&parser, {}, get_json_token(&parser));
    
    return result;
}

static void free_json(JsonElement* element) {
    while (element) {
        JsonElement* freeElement = element;
        element = element->nextSibling;
        
        free_json(freeElement->firstSubElement);
        free(freeElement);
    }
}

static JsonElement* lookup_element(JsonElement* object, String elementName) {
    JsonElement* result = 0;
    
    if (object) {
        for (JsonElement* search = object->firstSubElement; search; search = search->nextSibling) {
            if (are_equal(search->label, elementName)) {
                result = search;
                break;
            }
        }
    }
    
    return result;
}

static double convert_json_sign(String source, u64* atResult) {
    u64 at = *atResult;
    
    double result = 1.0;
    if (is_in_bound(source, at) && (source.data[at] == '-')) {
        result = -1.0;
        ++at;
    }
    
    *atResult = at;
    
    return result;
}

static double convert_json_number(String source, u64* atResult) {
    u64 at = *atResult;
    
    double result = 0.0;
    
    while (is_in_bound(source, at)) {
        u8 character = source.data[at] - (u8)'0';
        if (character < 10) {
            result = 10.0 * result + (double)character;
            ++at;
        } else {
            break;
        }
    }
    
    *atResult = at;
    
    return result;
}

static double convert_element_to_double(JsonElement* object, String elementName) {
    double result = 0.0;
    
    JsonElement* element = lookup_element(object, elementName);
    if (element) {
        String source = element->value;
        u64 at = 0;
        
        double sign = convert_json_sign(source, &at);
        double number = convert_json_number(source, &at);
        
        if (is_in_bound(source, at) && (source.data[at] == '.')) {
            ++at;
            double c = 1.0 / 10.0;
            while (is_in_bound(source, at)) {
                u8 character = source.data[at] - (u8)'0';
                if (character < 10) {
                    number = number + c * (double)character;
                    c *= 1.0 / 10.0;
                    ++at;
                } else {
                    break;
                }
            }
        }
        
        if (is_in_bound(source, at) && ((source.data[at] == 'e') || (source.data[at] == 'E'))) {
            ++at;
            if (is_in_bound(source, at) && (source.data[at] == '+')) {
                ++at;
            }
            
            double exponentSign = convert_json_sign(source, &at);
            double exponent = exponentSign * convert_json_number(source, &at);
            number *= pow(10.0, exponent);
        }
        
        result = sign * number;
    }
    
    return result;
}

static u64 parse_haversine_pairs(String inputJson, u64 maxPairCount, HaversinePair* pairs) {
    PROFILE_FUNC();
    
    u64 pairCount = 0;
    
    JsonElement* json = parse_json(inputJson);
    JsonElement* pairsArray = lookup_element(json, CONSTANT_STRING("pairs"));
    
    if (pairsArray) {
        PROFILE_SCOPE("Lookup and Convert");
        for (JsonElement* element = pairsArray->firstSubElement;
             element && (pairCount < maxPairCount);
             element = element->nextSibling) {
            HaversinePair* pair = pairs + pairCount++;
            
            pair->x0 = convert_element_to_double(element, CONSTANT_STRING("x0"));
            pair->y0 = convert_element_to_double(element, CONSTANT_STRING("y0"));
            pair->x1 = convert_element_to_double(element, CONSTANT_STRING("x1"));
            pair->y1 = convert_element_to_double(element, CONSTANT_STRING("y1"));
        }
    }
    
    free_json(json);
    
    return pairCount;
}