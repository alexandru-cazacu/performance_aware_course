#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h> // _stat64

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define U64Max UINT64_MAX

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

struct HaversinePair {
    double x0, x1;
    double y0, y1;
};

#define PROFILER 1
#define PROFILER_BLOCK_TIMER read_cpu_timer

#include "metrics.cpp"
#include "profiler.cpp"
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

static String read_file(char* path) {
    PROFILE_FUNC();
    
    String result = {};
    
    FILE* file = fopen(path, "rb");
    
    if (file == nullptr) {
        return result;
    }
    
#if _WIN32
    struct __stat64 stat;
    _stat64(path, &stat);
#else
    struct stat Stat;
    stat(path, &stat);
#endif
    
    result = allocate_string(stat.st_size);
    
    if (result.data) {
        if (fread(result.data, result.count, 1, file) != 1) {
            // Can't read file
            free_string(&result);
        }
    }
    
    fclose(file);
    
    return result;
}

static double sum_haversine_distances(u64 pairCount, HaversinePair* pairs) {
    PROFILE_FUNC();
    
    double sum = 0;
    
    double sumCoeff = 1 / (double)pairCount;
    for (u64 i = 0; i < pairCount; i++) {
        HaversinePair pair = pairs[i];
        double earthRadius = 6372.8;
        double dist = reference_haversine(pair.x0, pair.y0, pair.x1, pair.y1, earthRadius);
        sum += sumCoeff * dist;
    }
    
    return sum;
}

// [haversine_input.json]
// [haversine_input.json] [answers.double]
int main(int argc, char** argv) {
    begin_profile();
    
    int result = 1;
    
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
    
    String inputJson = read_file(jsonFilePath);
    
    if (inputJson.data == nullptr) {
        fprintf(stderr, "Can't open JSON file \"%s\"", jsonFilePath);
        return 0;
    }
    
    if (argc >= 3) {
        String answers = read_file(answersFilePath);
        
        if (answers.data == nullptr) {
            fprintf(stderr, "Can't open answers file \"%s\"", answersFilePath);
            return 0;
        }
    }
    
    u32 minimumJsonPairEncoding = 6 * 4;
    u64 maxPairCount = inputJson.count / minimumJsonPairEncoding;
    if (maxPairCount) {
        String parsedValues = allocate_string(maxPairCount * sizeof(HaversinePair));
        if (parsedValues.count) {
            HaversinePair* pairs = (HaversinePair*)parsedValues.data;
            u64 pairCount = parse_haversine_pairs(inputJson, maxPairCount, pairs);
            double sum = sum_haversine_distances(pairCount, pairs);
            
            fprintf(stdout, "Input size: %llu\n", inputJson.count);
            fprintf(stdout, "Pair count: %llu\n", pairCount);
            fprintf(stdout, "Haversine sum: %.16f\n", sum);
            
            if (argc == 3) {
                String answersDouble = read_file(argv[2]);
                if (answersDouble.count >= sizeof(double)) {
                    
                    double* answerValues = (double*)answersDouble.data;
                    
                    fprintf(stdout, "\nValidation:\n");
                    
                    u64 refAnswerCount = (answersDouble.count - sizeof(double)) / sizeof(double);
                    if (pairCount != refAnswerCount) {
                        fprintf(stdout, "FAILED - pair count doesn't match %llu.\n", refAnswerCount);
                    }
                    
                    double refSum = answerValues[refAnswerCount];
                    
                    fprintf(stdout, "Reference sum: %.16f\n", refSum);
                    fprintf(stdout, "Difference: %.16f\n", sum - refSum);
                    
                    fprintf(stdout, "\n");
                }
            }
        }
        
        free_string(&parsedValues);
    } else {
        fprintf(stderr, "Malformed input JSON\n");
    }
    
    free_string(&inputJson);
    
    end_profile_and_print();
    
    result = 0;
    
    return result;
}

PROFILER_ASSERT;