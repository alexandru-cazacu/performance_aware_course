#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

static double square(double l) {
    double result = l * l;
    return result;
}

static double radiansFromDegrees(double Degrees) {
    double Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static double referenceHaversine(double x0, double y0, double x1, double y1, double earthRadius) {
    double lat1 = y0;
    double lat2 = y1;
    double lon1 = x0;
    double lon2 = x1;
    
    double dLat = radiansFromDegrees(lat2 - lat1);
    double dLon = radiansFromDegrees(lon2 - lon1);
    lat1 = radiansFromDegrees(lat1);
    lat2 = radiansFromDegrees(lat2);
    
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
int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: %s [uniform/cluster] [seed] [num. of pairs to generate]\n", argv[0]);
        return 1;
    }
    
    char* filePath = argv[1];
    
    return 0;
}