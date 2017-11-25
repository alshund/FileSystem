#include <iostream>
#include <windows.h>
#include <cmath>
#include <sstream>
#include <assert.h>


#define CREATE_FILE_ERROR 1;
#define GET_FILE_SIZE_ERROR 2;
#define CREATE_FILE_MAPPING_ERROR 3;
#define MAP_VIEW_OF_FILE_ERROR 4;
#define LACK_OF_MEMORY 5;
#define WRONG_FILENAME -3;
#define FILE_ALREADY_EXIST -2;
#define FILE_NOT_FOUND -1;
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

    int createFile(const char *filename) {
        int response = findFileIndex(filename);

        if (response == -3)
            return WRONG_FILENAME;

        if (response != -1) {
            return FILE_ALREADY_EXIST;
        }


        unsigned int index;
        unsigned int file_index;
        unsigned int previous_index = index;
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
        file_index = index;
        index = 0;
        while (true) {
            previous_index = index;
            index = fileSystemData[index].getNext();
            if (index == 0) {
                index = previous_index;
                break;
            }
        }

        for (int i = 0; i < blocksAmount; i++) {
            if (fileSystemData[i].isEmpty()) {
                previous_index = index;
                index = i;
                empty_block_found = true;
                break;
            }
        }

        if (empty_block_found) {
            fileSystemData[previous_index].setNext(index);
            memcpy(fileSystemData[index].ptr + DataBlock::pointer_size,
                   filename, DataBlock::block_size - DataBlock::pointer_size);
            memcpy(fileSystemData[index].ptr + DataBlock::block_size, &file_index, DataBlock::pointer_size);
            return SUCCESSFUL_IMPLEMENTATION;
        } else return LACK_OF_MEMORY;

    }

    std::string read(unsigned int start_block_index) {

        std::string data_buffer;
        DataBlock data_block;
        unsigned int block_index = fileSystemData[start_block_index].getNext();
        do {
            data_block = fileSystemData[block_index];
            data_buffer += data_block.read();
            block_index = data_block.getNext();
        } while (block_index != 0 || fileSystemData[block_index].isEmpty());
        return data_buffer;
    }

    int findFileIndex(const char *file_name) {
        if (strlen(file_name) > 8) {
            return WRONG_FILENAME;
        }

        unsigned int index = 0;
        unsigned int *indexBuffer = (unsigned int *) malloc(DataBlock::pointer_size);
        char *buffer = new char[DataBlock::block_size - DataBlock::pointer_size];
        unsigned int previous_index = 0;
        while (true) {
            index = fileSystemData[index].getNext();
            if (index == 0) break;
            memcpy(buffer, fileSystemData[index].ptr + DataBlock::pointer_size,
                   DataBlock::block_size - DataBlock::pointer_size);
            if (strcmp(file_name, buffer) == 0) {
                memcpy(indexBuffer, fileSystemData[index].ptr + DataBlock::block_size, DataBlock::pointer_size);
                return *indexBuffer;
            }

        };

        return FILE_NOT_FOUND;
    }

    std::string readFromFile(const char *file_name) {
        int index = findFileIndex(file_name);
        if (index == -1) return "";
        return read(index);
    }

    int writeToFile(const char *file_name, const char *input) {
        int index = findFileIndex(file_name);
        if (index == -3) return WRONG_FILENAME;
        if (index == -1) return FILE_NOT_FOUND;
        return write(index, input);
    }

    int copyFile(const char *file_name, const char *new_file_name) {

        int index = findFileIndex(file_name);
        if (index == -3) return WRONG_FILENAME;

        if (index == -1)
            return FILE_NOT_FOUND;

        int response = findFileIndex(new_file_name);
        if (response == -3)
            return WRONG_FILENAME;

        if (response != -1)
            return FILE_ALREADY_EXIST;

        createFile(new_file_name);

        writeToFile(new_file_name, read(index).c_str());
        return SUCCESSFUL_IMPLEMENTATION;

    }

    int renameFile(const char *file_name, const char *new_file_name) {
        int response = findFileIndex(new_file_name);
        if (response == -3)
            return WRONG_FILENAME;

        if (response != -1)
            return FILE_ALREADY_EXIST;

        unsigned int index = 0;
        unsigned int *indexBuffer = (unsigned int *) malloc(DataBlock::pointer_size);
        char *buffer = new char[DataBlock::block_size - DataBlock::pointer_size];
        unsigned int previous_index = 0;
        while (true) {
            index = fileSystemData[index].getNext();
            if (index == 0) break;
            memcpy(buffer, fileSystemData[index].ptr + DataBlock::pointer_size,
                   DataBlock::block_size - DataBlock::pointer_size);
            if (strcmp(file_name, buffer) == 0) {
                memcpy(indexBuffer, fileSystemData[index].ptr + DataBlock::block_size, DataBlock::pointer_size);
                fileSystemData[*indexBuffer].write(new_file_name);
                memcpy(fileSystemData[index].ptr + DataBlock::pointer_size, new_file_name,
                       DataBlock::block_size - DataBlock::pointer_size);
                return SUCCESSFUL_IMPLEMENTATION;
            }

        }

        return FILE_NOT_FOUND;
    }

    int deleteFile(const char *file_name) {
        int response = findFileIndex(file_name);
        unsigned int index = 0;
        unsigned int previous_index = 0;
        unsigned int next_index = 0;
        char *buffer = new char[DataBlock::block_size - DataBlock::pointer_size];
        unsigned int *indexBuffer = (unsigned int *) malloc(DataBlock::pointer_size);

        char chunk[DataBlock::block_size + DataBlock::pointer_size];
        memset(chunk, 0, DataBlock::block_size + DataBlock::pointer_size);

        if (response == -3)
            return WRONG_FILENAME;

        if (response == -1)
            return FILE_NOT_FOUND;

        while (true) {
            previous_index = index;
            index = fileSystemData[index].getNext();
            if (index == 0) break;

            memcpy(buffer, fileSystemData[index].ptr + DataBlock::pointer_size,
                   DataBlock::block_size - DataBlock::pointer_size);

            if (strcmp(file_name, buffer) == 0) {
                next_index = fileSystemData[index].getNext();

                memcpy(indexBuffer, fileSystemData[index].ptr + DataBlock::block_size, DataBlock::pointer_size);

                memcpy(fileSystemData[index].ptr, chunk, DataBlock::block_size + DataBlock::pointer_size);

                fileSystemData[previous_index].setNext(next_index);
                return deleteFileData(*indexBuffer);
            }

        };
        return FILE_NOT_FOUND;
    }

    int deleteFileData(unsigned int index) {
        unsigned int next_index = 0;
        char chunk[DataBlock::block_size + DataBlock::pointer_size];
        memset(chunk, 0, DataBlock::block_size + DataBlock::pointer_size);
        while (true) {
            next_index = fileSystemData[index].getNext();
            memcpy(fileSystemData[index].ptr, chunk, DataBlock::block_size + DataBlock::pointer_size);
            index = next_index;
            if (index == 0) break;
        }

        return SUCCESSFUL_IMPLEMENTATION;
    }

    void showAllFiles() {
        unsigned int index = 0;
        char *buffer = new char[DataBlock::block_size - DataBlock::pointer_size];
        std::string file_name;
        while (true) {
            index = fileSystemData[index].getNext();
            if (index == 0) break;
            memcpy(buffer, fileSystemData[index].ptr + DataBlock::pointer_size,
                   DataBlock::block_size - DataBlock::pointer_size);
            file_name = buffer;
            std::cout << file_name << "\n";
        }
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

    std::string test = "test1";
    std::string test2 = "test3";
    std::string new_test2 = "CatDog";
    fileSystem->write(1, temp2.c_str());


    fileSystem->createFile("test");
    fileSystem->createFile("test1");
    fileSystem->createFile("onemore");
    fileSystem->createFile("qwer");

    fileSystem->writeToFile(test.c_str(), temp2.c_str());

    std::cout << fileSystem->readFromFile(test.c_str()) << "\n";

    fileSystem->copyFile(test.c_str(), test2.c_str());

    std::cout << fileSystem->readFromFile(test2.c_str()) << "\n";
    fileSystem->showAllFiles();
    std::cout << "-------------------------" << "\n";
    fileSystem->renameFile(test2.c_str(), new_test2.c_str());

    std::cout << fileSystem->readFromFile(new_test2.c_str()) << "\n";

    fileSystem->showAllFiles();

    fileSystem->deleteFile(new_test2.c_str());

    std::cout << "-------------------------" << "\n";

    fileSystem->showAllFiles();
    std::cout << "-------------------------" << "\n";
    std::cout << fileSystem->readFromFile(test.c_str()) << "\n";
    fileSystem->createFile("qwerty");
    std::cout << "-------------------------" << "\n";

    fileSystem->showAllFiles();

    int response = fileSystem->findFileIndex(test2.c_str());
    std::cout<<response<<"\n";

    delete fileSystem;
    return 0;
}