#include <fcntl.h>
#include <io.h>

struct ReadParams {
    String dest;
    const char* fileName;
};

typedef void read_overhead_test_func(RepetitionTester* tester, ReadParams* params);

static void read_via_fread(RepetitionTester* tester, ReadParams* params) {
    while (tester_is_testing(tester)) {
        FILE* file = fopen(params->fileName, "rb");
        if (file) {
            String destBuffer = params->dest;
            
            tester_begin_time(tester);
            size_t result = fread(destBuffer.data, destBuffer.count, 1, file);
            tester_end_time(tester);
            
            if (result == 1) {
                count_bytes(tester, destBuffer.count);
            } else{
                tester_error(tester, "fread failed");
            }
            
            fclose(file);
        } else {
            tester_error(tester, "fopen failed");
        }
    }
}

static void read_via_read(RepetitionTester* tester, ReadParams* params) {
    while (tester_is_testing(tester)) {
        int file = _open(params->fileName, _O_BINARY | _O_RDONLY);
        if (file != -1) {
            String destBuffer = params->dest;
            
            u8* dest = destBuffer.data;
            u64 sizeRemaining = destBuffer.count;
            
            while (sizeRemaining) {
                u32 readSize = INT_MAX;
                if ((u64)readSize > sizeRemaining) {
                    readSize = (u32)sizeRemaining;
                }
                
                tester_begin_time(tester);
                int result = _read(file, dest, readSize);
                tester_end_time(tester);
                
                if (result == (int)readSize) {
                    count_bytes(tester, readSize);
                } else{
                    tester_error(tester, "_read failed");
                    break;
                }
                
                sizeRemaining -= readSize;
                dest += readSize;
            }
            
            _close(file);
        } else {
            tester_error(tester, "_open failed");
        }
    }
}

static void read_via_read_file(RepetitionTester* tester, ReadParams* params) {
    while (tester_is_testing(tester)) {
        HANDLE file = CreateFileA(params->fileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != INVALID_HANDLE_VALUE) {
            String destBuffer = params->dest;
            
            u64 sizeRemaining = params->dest.count;
            u8* dest = (u8*)destBuffer.data;
            
            while (sizeRemaining) {
                u32 readSize = (u32)-1;
                if ((u64)readSize > sizeRemaining) {
                    readSize = (u32)sizeRemaining;
                }
                
                DWORD bytesRead = 0;
                tester_begin_time(tester);
                int result = ReadFile(file, dest, readSize, &bytesRead, 0);
                tester_end_time(tester);
                
                if (result && (bytesRead == readSize)) {
                    count_bytes(tester, readSize);
                } else{
                    tester_error(tester, "ReadFile failed");
                }
                
                sizeRemaining -= readSize;
                dest += readSize;
            }
            
            CloseHandle(file);
        } else {
            tester_error(tester, "CreateFileA failed");
        }
    }
}