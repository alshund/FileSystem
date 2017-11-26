
#include <iostream>
#include "FileSystem.h"


int main() {

    FileSystem *fileSystem = new FileSystem();
    fileSystem->initialize(8000);
    fileSystem->createFile("mama");
    fileSystem->createFile("papa");
    fileSystem->createFile("as");
    fileSystem->write("mama", "1234567890qweqweqweqevvfvfvfvffvfvfvfvfvfv");
    std::cout << fileSystem->read("mama") << std::endl;
    fileSystem->showAllFiles();

    return 0;
}