#include <iostream>
#include <windows.h>


#define CREATE_FILE_ERROR 1;
#define GET_FILE_SIZE_ERROR 2;
#define CREATE_FILE_MAPPING_ERROR 3;
#define MAP_VIEW_OF_FILE_ERROR 4;


struct FileMapping {
    HANDLE hFile;
    HANDLE hFileMapping;
    size_t fileSize;
    unsigned char *fileMappingPtr;
};

size_t block_size = 5 * sizeof(unsigned int);
size_t pointer_size = sizeof(unsigned int);

struct DataBlock {
    unsigned char *ptr;
    unsigned int next_index;
    bool empty;
};

FileMapping *filMapping = (FileMapping *) malloc(sizeof(FileMapping));
DataBlock *fileSystemData;

int init(size_t prefered_size) {
    int blocksAmount = prefered_size / block_size;
    size_t size = blocksAmount*block_size;

    HANDLE hFile = CreateFile("file_system",
                              GENERIC_READ | GENERIC_WRITE,
                              0, //process access to the file = nothing
                              nullptr,//security attribute
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return CREATE_FILE_ERROR;
    }
    char chunk[size];
    memset(chunk, 0, size);
    DWORD fileSize;
    WriteFile(hFile, chunk, sizeof(chunk), &fileSize, nullptr);
    DWORD dwFileSize = GetFileSize(hFile, nullptr); //nullptr - max file size = 4Gb
    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return GET_FILE_SIZE_ERROR;
    }

    HANDLE hFileMapping = CreateFileMapping(hFile,
                                            nullptr,
                                            PAGE_READWRITE,
                                            0,
                                            size,
                                            nullptr);
    if (hFileMapping == nullptr) {
        CloseHandle(hFile);
        return CREATE_FILE_MAPPING_ERROR;
    }

    unsigned char *fileMappingPtr = (unsigned char *) MapViewOfFile(hFileMapping,
                                                                    FILE_MAP_WRITE,
                                                                    0,
                                                                    0,
                                                                    dwFileSize);
    if (fileMappingPtr == nullptr) {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return MAP_VIEW_OF_FILE_ERROR;
    }


    filMapping->hFile = hFile;
    filMapping->hFileMapping = hFileMapping;
    filMapping->fileMappingPtr = fileMappingPtr;
    filMapping->fileSize = (size_t) dwFileSize;

    fileSystemData = new DataBlock[blocksAmount];
    fileSystemData[0].ptr = filMapping->fileMappingPtr;
    fileSystemData[0].next_index = 0;
    fileSystemData[0].empty = false;

    for(int i = 1; i < blocksAmount; i++){
        fileSystemData[i].ptr = filMapping->fileMappingPtr + i*block_size;
        fileSystemData[i].next_index = 0;
        fileSystemData[i].empty = true;
    }
}

int main() {

    init(8000);


    std::cout << filMapping->fileSize;
    return 0;
}