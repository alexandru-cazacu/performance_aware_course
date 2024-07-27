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

struct RandomSeries {
    u64 a;
    u64 b;
    u64 c;
    u64 d;
};

static u64 rotate_left(u64 v, int shift) {
    u64 result = ((v << shift) | (v >> (64 - shift)));
    return result;
}

static u64 random_u64(RandomSeries* series) {
    u64 a = series->a;
    u64 b = series->b;
    u64 c = series->c;
    u64 d = series->d;
    
    u64 e = a - rotate_left(b, 27);
    
    a = (b ^ rotate_left(c, 17));
    b = (c + d);
    c = (d + e);
    d = (e + a);
    
    series->a = a;
    series->b = b;
    series->c = c;
    series->d = d;
    
    return d;
}

static RandomSeries seed(u64 value) {
    RandomSeries series = {};
    
    // NOTE(alex): JSF generators
    series.a = 0xf1ea5eed;
    series.b = value;
    series.c = value;
    series.d = value;
    
    u32 count = 20;
    while(count--) {
        random_u64(&series);
    }
    
    return series;
}

static double random_in_range(RandomSeries* series, double min, double max) {
    double t = (double)random_u64(series) / (double)U64Max;
    double result = (1.0 - t) * min + t * max;
    
    return result;
}

static double random_degree(RandomSeries* series, double center, double radius, double maxAllowed) {
    double minVal = center - radius;
    if (minVal < -maxAllowed) {
        minVal = -maxAllowed;
    }
    
    double maxVal = center + radius;
    if (maxVal > maxAllowed) {
        maxVal = maxAllowed;
    }
    
    double result = random_in_range(series, minVal, maxVal);
    return result;
}

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
    
    fseek(file, 0, SEEK_END);
    
    int size = ftell(file);
    
    fseek(file, 0, SEEK_SET);
    
    result.size = size;
    
    result.data = (u8*)malloc(sizeof(u8) * result.size);
    fread(result.data, result.size, 1, file);
    
    fclose(file);
    
    return result;
}

static FILE* file_open(u64 pairCount, const char* label, const char* extension) {
    char temp[256];
    sprintf(temp, "data_%llu_%s.%s", pairCount, label, extension);
    FILE* result = fopen(temp, "wb");
    if (!result) {
        fprintf(stderr, "Unable to open \"%s\" for writing.\n", temp);
    }
    
    return result;
}

// [uniform/cluster] [seed] [num. of pairs to generate]
// uniform -> whole sphere
// cluser -> randomized square clusers to avoid total sum. convergence
int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [uniform/cluster] [seed] [num. of pairs to generate]\n", argv[0]);
        return 1;
    }
    
    u64 clusterCountLeft = U64Max;
    
    char* methodName = argv[1];
    if (strcmp(methodName, "cluster") == 0) {
        clusterCountLeft = 0;
    } else if (strcmp(methodName, "uniform") != 0) {
        methodName = "uniform";
        fprintf(stderr, "WARNING: Unrecognized method name. Using 'uniform'.\n");
    }
    
    u64 maxPairCount = (1ULL << 34);
    u64 pairCount = atoll(argv[3]);
    
    if (pairCount >= maxPairCount) {
        fprintf(stderr, "To avoid accidentally generating massive files, number of pairs must be less than %llu.\n", maxPairCount);
        return 1;
    }
    
    FILE* json = file_open(pairCount, "flex", "json");
    FILE* binaryAnswers = file_open(pairCount, "binaryanswers", "bin");
    
    u64 seedValue = atoll(argv[2]);
    RandomSeries series = seed(seedValue);
    
    double maxAllowedX = 180;
    double maxAllowedY = 90;
    
    double xCenter = 0;
    double yCenter = 0;
    double xRadius = maxAllowedX;
    double yRadius = maxAllowedY;
    
    u64 clusterCountMax = 1 + (pairCount / 64);
    
    if (json && binaryAnswers) {
        double sum = 0;
        double sumCoeff = 1.0 / (double)pairCount;
        
        fprintf(json, "{\"pairs\":[\n");
        
        for (u64 i = 0; i < pairCount; ++i) {
            if(clusterCountLeft-- == 0) {
                clusterCountLeft = clusterCountMax;
                xCenter = random_in_range(&series, -maxAllowedX, maxAllowedX);
                yCenter = random_in_range(&series, -maxAllowedY, maxAllowedY);
                xRadius = random_in_range(&series, 0, maxAllowedX);
                yRadius = random_in_range(&series, 0, maxAllowedY);
            }
            
            double x0 = random_degree(&series, xCenter, xRadius, maxAllowedX);
            double y0 = random_degree(&series, yCenter, yRadius, maxAllowedY);
            double x1 = random_degree(&series, xCenter, xRadius, maxAllowedX);
            double y1 = random_degree(&series, yCenter, yRadius, maxAllowedY);
            
            double earthRadius = 6372.8;
            double haverDistance = reference_haversine(x0, y0, x1, y1, earthRadius);
            
            sum += sumCoeff * haverDistance;
            
            const char* jsonSep = (i == (pairCount - 1)) ? "\n" : ",\n";
            fprintf(json, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}%s", x0, y0, x1, y1, jsonSep);
            
            fwrite(&haverDistance, sizeof(haverDistance), 1, binaryAnswers);
        }
        
        fprintf(json, "]}\n");
        fwrite(&sum, sizeof(sum), 1, binaryAnswers);
        
        fprintf(stdout, "Method: %s\n", methodName);
        fprintf(stdout, "Random seed: %llu\n", seedValue);
        fprintf(stdout, "Pair count: %llu\n", pairCount);
        fprintf(stdout, "Expected sum: %.16f\n", sum);
    }
    
    if (json) {
        fclose(json);
    }
    if (binaryAnswers) {
        fclose(binaryAnswers);
    }
    
    return 0;
}