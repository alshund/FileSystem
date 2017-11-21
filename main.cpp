#include <iostream>
#include <windows.h>
#include <string.h>


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


class DataBlock {
public:
    const static size_t block_size = 12 * sizeof(char);
    const static size_t pointer_size = sizeof(unsigned int);

    unsigned char *ptr;

    void write(const char *buffer) {
        memcpy(ptr + pointer_size, buffer, block_size);
    }

    void setNext(unsigned int next) {
        memcpy(ptr, &next, pointer_size);
    }

    unsigned int getNext() {
        unsigned int *indexBuffer = (unsigned int *) malloc(pointer_size);
        memcpy(indexBuffer, ptr, pointer_size);
        return *indexBuffer;
    }

    char *read() {
        char *pBuffer = (char *) malloc(block_size);
        memcpy(pBuffer, ptr + pointer_size, block_size);
        return pBuffer;
    }

    bool isEmpty(){
        char *pBuffer = (char *) malloc(block_size);
        memcpy(pBuffer, ptr + pointer_size, block_size);
        std::string tmp = pBuffer;
        return tmp == "";
    }
};


class FileSystem{
public:
    FileMapping *fileMapping = (FileMapping *) malloc(sizeof(FileMapping));
    DataBlock *fileSystemData;

    int init(size_t prefered_size) {
        int blocksAmount = prefered_size / (DataBlock::block_size + DataBlock::pointer_size);
        size_t size = blocksAmount *  (DataBlock::block_size + DataBlock::pointer_size);

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

        for (int i = 0; i < blocksAmount; i++) {
            fileSystemData[i].ptr = fileMapping->dataPtr + i *  (DataBlock::block_size + DataBlock::pointer_size);
        }
        return 0;
    }

    std::string read(unsigned int start_block_index) {

        std::string data_buffer;
        DataBlock data_block;
        unsigned int block_index = start_block_index;
        do {
            data_block = fileSystemData[block_index];
            data_buffer += data_block.read();
            block_index = data_block.getNext();
        } while(block_index != 0 || fileSystemData[block_index].isEmpty());
        return data_buffer;
    }

    ~FileSystem(){
        UnmapViewOfFile(fileMapping->dataPtr);
        CloseHandle(fileMapping->hFileMapping);
        CloseHandle(fileMapping->hFile);
        free(fileMapping);
    }
};

int main() {

    FileSystem *fileSystem = new FileSystem();

    fileSystem -> init(8000);

    std::string temp2 = "fat";


    fileSystem ->fileSystemData[0].write(temp2.c_str());
    std::string test;
    test = fileSystem ->fileSystemData[0].read();
    std::cout << test<<"\n";
    std::cout << fileSystem ->fileSystemData[0].isEmpty()<<"\n";
    std::cout << fileSystem ->fileSystemData[5].isEmpty()<<"\n";

    fileSystem->fileSystemData[0].setNext(5);
    fileSystem->fileSystemData[5].write("mamk");
    std::cout << fileSystem->fileSystemData[5].read() << std::endl;
//    std::cout << fileSystem ->fileSystemData[0].getNext();

    std::cout << fileSystem->read(0) << std::endl;

    char *buffer_1 = const_cast<char *>("123456789");
    char *buffer_2 = "";
    memcpy(buffer_2, buffer_1, 2);
    std::cout << buffer_2 << std::endl;


    delete fileSystem;
    return 0;
}