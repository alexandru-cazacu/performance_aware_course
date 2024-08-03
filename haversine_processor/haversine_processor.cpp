#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define U64Max UINT64_MAX

struct HaversinePair {
    double x0, x1;
    double y0, y1;
};

#include "string.cpp"
#include "json_parser.cpp"

static double square(double l) {
    double result = l * l;
    return result;
}

static double radians_from_degrees(double Degrees) {
    double Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(alex): EarthRadius is generally expected to be 6372.8
static double reference_haversine(double x0, double y0, double x1, double y1, double earthRadius) {
    double lat1 = y0;
    double lat2 = y1;
    double lon1 = x0;
    double lon2 = x1;
    
    double dLat = radians_from_degrees(lat2 - lat1);
    double dLon = radians_from_degrees(lon2 - lon1);
    lat1 = radians_from_degrees(lat1);
    lat2 = radians_from_degrees(lat2);
    
    double a = square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(dLon / 2));
    double c = 2.0 * asin(sqrt(a));
    
    double result = earthRadius * c;
    
    return result;
}

struct File {
    u8* data;
    int size;
};

static File file_read(char* path) {
    File result = {};
    
    FILE* file = fopen(path, "rb");
    
    if (file == nullptr) {
        return result;
    }
    
    fseek(file, 0, SEEK_END);
    
    int size = ftell(file);
    
    fseek(file, 0, SEEK_SET);
    
    result.size = size;
    
    result.data = (u8*)malloc(sizeof(u8) * result.size);
    fread(result.data, result.size, 1, file);
    
    fclose(file);
    
    return result;
}

// [haversine_input.json]
// [haversine_input.json] [answers.double]
int main(int argc, char** argv) {
    if (argc <= 1 || argc >= 4) {
        fprintf(stderr, "Usage: %s [haversine_input.json]\n", argv[0]);
        fprintf(stderr, "       %s [haversine_input.json] [answers.double]\n", argv[0]);
        return 1;
    }
    
    char* jsonFilePath = nullptr;
    char* answersFilePath = nullptr;
    
    if (argc >= 2) {
        jsonFilePath = argv[1];
    }
    
    if (argc >= 3) {
        answersFilePath = argv[2];
    }
    
    File json = file_read(jsonFilePath);
    
    if (json.data == nullptr) {
        fprintf(stderr, "Can't open JSON file \"%s\"", jsonFilePath);
        return 0;
    }
    
    if (argc >= 3) {
        File answers = file_read(answersFilePath);
        
        if (answers.data == nullptr) {
            fprintf(stderr, "Can't open answers file \"%s\"", answersFilePath);
            return 0;
        }
    }
    
    // TODO(alex): Parse json
    // TODO(alex): Compute haversine sum
    // TODO(alex): Validate sum using answers file
    
    u64 inputSize = 0;
    u64 pairCount = 0;
    double haversineSum = 0;
    double referenceSum = 0;
    double difference = 0;
    
    printf("Input size: %llu\n", inputSize);
    printf("Pair count: %llu\n", pairCount);
    printf("Haversine sum: %.16f\n\n", haversineSum);
    printf("Validation:\n");
    printf("Reference sum: %.16f\n", referenceSum);
    printf("Difference: %.16f\n", difference);
    
    return 0;
}