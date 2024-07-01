#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;

struct File {
    u8* data;
    int size;
};

static File file_read(char* path) {
    File result = {};
    
    FILE* file = fopen(path, "rb");
    
    fseek(file, 0, SEEK_END);
    
    int size = ftell(file);
    
    result.size = size;
    
    fclose(file);
    
    return result;
}

int main(void) {
    File file = file_read("file.txt");
    
    printf("Size: %d\n", file.size);
    
    return 0;
}