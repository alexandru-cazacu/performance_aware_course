#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct String String;
struct String
{
  u8* data;
  u32 len;
};

String OS_ReadFile(const char* path)
{
  String result = { 0 };
  FILE* file = fopen(path, "rb");
  if (file)
  {
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    u8* data = malloc(size + 1); // +1 for nul
    u32 read = fread(data, 1, size, file);
    if (read == size)
    {
      data[size] = '\0';
      result.data = data;
      result.len = size;
    }
    else
    {
      free(data);
    }
  }
  fclose(file);
  return result;
}

#define MOD_MEM_MODE_NO_DISP 0b00
#define MOD_MEM_MODE_8_DISP  0b01
#define MOD_MEM_MODE_16_DISP 0b10
#define MOD_MEM_REG_NO_DISP  0b11

typedef enum Register Register;
enum Register
{
  AL,
  BL,
  CL,
  DL,
  AH,
  BH,
  CH,
  DH,
  AX,
  BX,
  CX,
  DX,
  SP,
  BP,
  SI,
  DI,
};

typedef struct Mod11TableEntry Mod11TableEntry;
struct Mod11TableEntry
{
  u8 w[2];
};

const char* regNames[] =
{
  "al",
  "bl",
  "cl",
  "dl",
  "ah",
  "bh",
  "ch",
  "dh",
  "ax",
  "bx",
  "cx",
  "dx",
  "sp",
  "bp",
  "si",
  "di",
};

int main(i32 argc, char* argv[])
{
  if (argc != 2)
  {
    printf("Usage:\n");
    printf("  %s <path>\n", argv[0]);
    return 1;
  }

  Mod11TableEntry entries[8] = {
    { AL, AX },
    { CL, CX },
    { DL, DX },
    { BL, BX },
    { AH, SP },
    { CH, BP },
    { DH, SI },
    { BH, DI },
  };

  String bin = OS_ReadFile(argv[1]);
  if (bin.data)
  {
    printf("; %s\n", argv[1]);
    printf("bits 16\n");

    for (int i = 0; i < bin.len; i++)
    {
      if (i + 1 < bin.len)
      {
        // 0 byte, 1 word
        u8 w = bin.data[i] >> 0 & 0b00000001;
        // 0 REG is src, 1 REG is dst
        u8 d = bin.data[i] >> 1 & 0b00000001;
        u8 opcode = bin.data[i] >> 2 & 0b00111111;
        
        u8 r_m = bin.data[i + 1] >> 0 & 0b00000111;
        u8 reg = bin.data[i + 1] >> 3 & 0b00000111;
        u8 mod = bin.data[i + 1] >> 6 & 0b00000011;
  
        switch (opcode)
        {
          case 0b100010: // MOV
          {
            switch (mod)
            {
              case MOD_MEM_REG_NO_DISP:
              {
                Register dst = entries[r_m].w[w];
                Register src = entries[reg].w[w];
  
                printf("mov %s, %s\n", regNames[dst], regNames[src]);
              } break;
              default:
              {
  
              } break;
            }
          } break;
          default:
          {
  
          } break;
        }

        i++;
      }
    }
  }
  else
  {
    fprintf(stderr, "%s not found\n", argv[1]);
    return 1;
  }

  return 0;
}