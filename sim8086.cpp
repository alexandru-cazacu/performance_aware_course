#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

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

char registers[][2][3]= {
    {"al", "ax"},
    {"cl", "cx"},
    {"dl", "dx"},
    {"bl", "bx"},
    {"ah", "sp"},
    {"ch", "bp"},
    {"dh", "si"},
    {"bh", "di"},
};

static void disabbemble(File file) {
    printf("bits 16\n");
    
    for (int i = 0; i < file.size; ++i) {
        u8 byte0 = file.data[i];
        
        // d - 0 reg is not dest, 1 reg is dest
        // w - 0 is 8 bits, 1 is 16 bits
        // mod - 11 - register to register
        // register - register
        // r/m - register or memory based on mod
        if ((byte0 >> 2) == 0b100010) {
            u8 d = byte0 >> 1 & 0b1;
            u8 w = byte0 >> 0 & 0b1;
            
            u8 byte1 = file.data[++i];
            u8 mod = byte1 >> 6 & 0b11;
            u8 reg = byte1 >> 3 & 0b111;
            u8 rm  = byte1 >> 0 & 0b111;
            
            if (mod == 0b11) {
                char* dest = NULL;
                char* src = NULL;
                if (d) {
                    dest = registers[reg][w];
                    src = registers[rm][w];
                } else {
                    dest = registers[rm][w];
                    src = registers[reg][w];
                }
                
                printf("mov %.2s, %.2s\n", dest, src);
            }
        }
        
        // Immediate to register
        // 1011 w reg | data | data (w=1)
        else if ((byte0 >> 4) == 0b1011) {
            u8 w = byte0 >> 3 & 0b1;
            u8 reg = byte0 >> 0 & 0b111;
            
            u8 byte1 = file.data[++i];
            u16 value = byte1;
            
            if (w == 1) {
                u8 byte2 = file.data[++i];
                value = (byte2 << 8) | byte1;
            }
            
            char* dest = registers[reg][w];
            
            printf("mov %.2s, %u\n", dest, value);
        }
    }
}

int main(int argc, char** argv) {
    //char* program = argv[0];
    
    if (argc == 1) {
        printf("File path not provided.\n");
        exit(1);
    }
    
    char* filePath = argv[1];
    
    File file = file_read(filePath);
    
    disabbemble(file);
    
    return 0;
}