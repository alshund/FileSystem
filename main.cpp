#include <iostream>
#include <windows.h>
#include <cmath>
#include <sstream>


#define CREATE_FILE_ERROR 1;
#define GET_FILE_SIZE_ERROR 2;
#define CREATE_FILE_MAPPING_ERROR 3;
#define MAP_VIEW_OF_FILE_ERROR 4;
#define LACK_OF_MEMORY 5;
#define WRONG_FILENAME 7;
#define SUCCESSFUL_IMPLEMENTATION 0;


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
        pBuffer[block_size] = '\0';
        return pBuffer;
    }

    bool isEmpty() {
        char *pBuffer = (char *) malloc(block_size);
        memcpy(pBuffer, ptr + pointer_size, block_size);
        std::string tmp = pBuffer;
        return tmp == "";
    }
};


class FileSystem {
public:
    FileMapping *fileMapping = (FileMapping *) malloc(sizeof(FileMapping));
    DataBlock *fileSystemData;
    int blocksAmount;

    int init(size_t prefered_size) {
        blocksAmount = prefered_size / (DataBlock::block_size + DataBlock::pointer_size);
        size_t size = blocksAmount * (DataBlock::block_size + DataBlock::pointer_size);

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
            fileSystemData[i].ptr = fileMapping->dataPtr + i * (DataBlock::block_size + DataBlock::pointer_size);
        }
        fileSystemData[0].write("^init^");
        return SUCCESSFUL_IMPLEMENTATION;
    }

    int write(unsigned int index, const char *input) {
        if (index > blocksAmount)
            return LACK_OF_MEMORY;

        unsigned int input_size = strlen(input);
        unsigned int blocks_amount = ceil((double) input_size / (double) DataBlock::block_size);
        unsigned int previous_index;
        boolean empty_block_found = false;
        //  unsigned int offset = 0;

        char *buffer = (char *) malloc(DataBlock::block_size);

        while (true) {
            previous_index = index;
            index = fileSystemData[index].getNext();
            if (index == 0) {
                index = previous_index;
                break;
            }
        }

        for (int i = 0; i < blocks_amount; i++) {
            empty_block_found = false;

            for (int j = 0; j < this->blocksAmount; j++) {
                if (fileSystemData[j].isEmpty()) {
                    previous_index = index;
                    index = j;
                    empty_block_found = true;
                    break;
                }
            }

            if (!empty_block_found) return LACK_OF_MEMORY;

            memcpy(buffer, input + i * DataBlock::block_size, DataBlock::block_size);
            fileSystemData[index].write(buffer);
            fileSystemData[previous_index].setNext(index);

        }

        return SUCCESSFUL_IMPLEMENTATION;
    }

    int createFile(const char* filename){
        //TODO: add regexp here, name size longer than 6, $ and ^ are not available in name
        if(strlen(filename) > 6)
            return WRONG_FILENAME;

        unsigned int index;
        boolean empty_block_found = false;

        for (int i = 0; i < this->blocksAmount; i++) {
            if (fileSystemData[i].isEmpty()) {
                index = i;
                empty_block_found = true;
                break;
            }
        }

        if (!empty_block_found) return LACK_OF_MEMORY;

        fileSystemData[index].write(filename);
        std::string correct_name = filename;
        std::stringstream ss;
        ss << correct_name <<"$"<< index;
        write(0,ss.str().c_str());
        return SUCCESSFUL_IMPLEMENTATION;
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

    ~FileSystem() {
        UnmapViewOfFile(fileMapping->dataPtr);
        CloseHandle(fileMapping->hFileMapping);
        CloseHandle(fileMapping->hFile);
        free(fileMapping);
    }
};

int main() {

    FileSystem *fileSystem = new FileSystem();

    fileSystem->init(8000);

    std::string temp2 = "line2dogdogdogdocatcatcatcaemanmanmanmae";
    std::string temp1 = "line1dfgdfgdfgdfgdogdogdogdodfg";


//    fileSystem->fileSystemData[0].write(temp2.c_str());
//    std::string test;
//    test = fileSystem->fileSystemData[0].read();
//    std::cout << test << "\n";
//    std::cout << fileSystem->fileSystemData[0].isEmpty() << "\n";
//    std::cout << fileSystem->fileSystemData[5].isEmpty() << "\n";
//
//    fileSystem->fileSystemData[0].setNext(666);
//    std::cout << fileSystem->fileSystemData[0].getNext();

    //    fileSystem->fileSystemData[0].write(temp1.c_str());
//    fileSystem->fileSystemData[1].write(temp2.c_str());
//    test = fileSystem->fileSystemData[0].read();
//    test += fileSystem->fileSystemData[1].read();
    // std::cout << test << "\n";
    std::string test;

    fileSystem->write(1, temp2.c_str());


    test = fileSystem->fileSystemData[1].read();
    test += fileSystem->fileSystemData[2].read();
    test += fileSystem->fileSystemData[3].read();
    test += fileSystem->fileSystemData[4].read();
    std::cout << test << "\n";

    fileSystem->createFile("test");

    test = fileSystem->fileSystemData[0].read();
    test += fileSystem->fileSystemData[5].read();
    test += fileSystem->fileSystemData[6].read();
    std::cout << test << "\n";

    delete fileSystem;
    return 0;
}