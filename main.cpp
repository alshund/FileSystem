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
    unsigned char *dataPtr;
};

size_t block_size = 5 * sizeof(char);
size_t pointer_size = sizeof(char);

struct DataBlock {
    unsigned char *ptr;
    unsigned int next_index;
    bool empty;
};

FileMapping *fileMapping = (FileMapping *) malloc(sizeof(FileMapping));
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


    fileMapping->hFile = hFile;
    fileMapping->hFileMapping = hFileMapping;
    fileMapping->dataPtr = fileMappingPtr;
    fileMapping->fileSize = (size_t) dwFileSize;

    fileSystemData = new DataBlock[blocksAmount];
    fileSystemData[0].ptr = fileMapping->dataPtr;
    fileSystemData[0].next_index = 0;
    fileSystemData[0].empty = false;

    for(int i = 1; i < blocksAmount; i++){
        fileSystemData[i].ptr = fileMapping->dataPtr + i*block_size;
        fileSystemData[i].next_index = 0;
        fileSystemData[i].empty = true;
    }
}

int main() {

    init(8000);

    std::string temp1 = "catedgdfgdfg";
    char  *check = (char *) temp1.c_str();
    std::cout<< strlen(check)<<std::endl;

    std::string temp2 = "dog";

    memcpy(fileSystemData[0].ptr, temp1.c_str() , block_size);
    memcpy(fileSystemData[3].ptr, temp2.c_str(), block_size);

    char *pBuffer = (char *) malloc(block_size);
    
    memcpy(pBuffer, fileSystemData[0].ptr, block_size);
    std::string test = pBuffer;
    std::cout<< test<<std::endl;

    memcpy(pBuffer, fileSystemData[3].ptr, block_size);
    test = pBuffer;
    std::cout<< test;


    UnmapViewOfFile(fileMapping->dataPtr);
    CloseHandle(fileMapping->hFileMapping);
    CloseHandle(fileMapping->hFile);
    free(fileMapping);
    return 0;
}