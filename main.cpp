#include <iostream>
#include <windows.h>

#define CREATE_FILE_ERROR 1;
#define GET_FILE_SIZE_ERROR 2;
#define CREATE_FILE_MAPPING_ERROR 3;
#define MAP_VIEW_OF_FILE_ERROR 4;

struct FileMapping{
    HANDLE hFile;
    HANDLE hFileMapping;
    size_t fileSize;
    unsigned char* fileMappingPtr;
};

int main() {
    std::cout << "Hello, World!" << std::endl;

    HANDLE hFile = CreateFile("file_system.dat",
                              GENERIC_READ | GENERIC_WRITE,
                              0, //process access to the file = nothing
                              nullptr,//security attribute
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return CREATE_FILE_ERROR;
    }

    DWORD dwFileSize = GetFileSize(hFile, nullptr); //nullptr - max file size = 4Gb
    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return GET_FILE_SIZE_ERROR;
    }

    HANDLE hFileMapping = CreateFileMapping(hFile,
                                            nullptr,
                                            PAGE_READWRITE,
                                            0,
                                            512,
                                            nullptr);
    if (hFileMapping == nullptr) {
        CloseHandle(hFile);
        return CREATE_FILE_MAPPING_ERROR;
    }

    unsigned char* fileMappingPtr = (unsigned char*) MapViewOfFile(hFileMapping,
                                                                   FILE_MAP_WRITE,
                                                                   0,
                                                                   0,
                                                                   dwFileSize);
    if (fileMappingPtr == nullptr) {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return MAP_VIEW_OF_FILE_ERROR;
    }


    FileMapping *fileMapping = (FileMapping*) malloc(sizeof(FileMapping));
    fileMapping->hFile = hFile;
    fileMapping->hFileMapping = hFileMapping;
    fileMapping->fileMappingPtr = fileMappingPtr;
    fileMapping->fileSize = (size_t) dwFileSize;


    return 0;
}