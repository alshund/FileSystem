
#include <iostream>
#include <vector>
#include <assert.h>
#include "FileSystem.h"


void touchTest() {
    int error;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(80);
    assert(error == 0);

    error = fileSystem->createFile("dsfsdfsdfsdfsdfsdfsdf");
    assert(error == -2);
    error = fileSystem->createFile("test");
    assert(error == 0);
    error = fileSystem->createFile("test");
    assert(error == -4);
    error = fileSystem->createFile("test1");
    assert(error == 0);
    error = fileSystem->createFile("test2");
    assert(error == -1);
    delete fileSystem;
    std::cout << "touch tests passed\n";
}

void writeTest() {
    int error;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(80);
    assert(error == 0);

    error = fileSystem->createFile("test");
    assert(error == 0);
    error = fileSystem->write("testsdgdsgsdgsdgsd","ddsgdsgdsgdfddsgdsgdsgdf");
    assert( error == -2 );

    error = fileSystem->write("test1","ddsgdsgdsgdfddsgdsgdsgdf");
    assert( error == -3 );

    error = fileSystem->write("test","ddsgdsgdsgdfddsgdsgdsgdf");
    assert( error == 0 );

    error = fileSystem->write("test","ddsgdsgdsgdfddsgdsgdsgdf");
    assert( error == -1 );
    delete fileSystem;

    std::cout << "write test passed\n";
}

void initTest() {
    int error = 0;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(8000);
    assert(error == 0);
    error = fileSystem->initialize(8000);
    assert(error == 0);
    delete fileSystem;
    fileSystem = new FileSystem();
    error = fileSystem->initialize(8000);
    assert(error == 0);
    delete fileSystem;
    std::cout << "init tests passed\n";

}

void readTest() {
    std::string str;
    int error;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(80);
    assert(error == 0);

    str = fileSystem->read("gfdgdfgfdgggggggggggg");
    assert(str == "");
    str = fileSystem->read("test");
    assert(str == "");

    error = fileSystem->createFile("test");
    assert(error == 0);
    str = fileSystem->read("test");
    assert(str == "$empty file$");
    error = fileSystem->write("test","ddsgdsgdsgdfddsgdsgdsgdf");
    assert( error == 0 );
    str = fileSystem->read("test");
    assert(str == "ddsgdsgdsgdfddsgdsgdsgdf");
    delete  fileSystem;
    std::cout << "read tests passed\n";
}

void renameTest() {
    int error;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(800);
    assert(error == 0);
    error = fileSystem->createFile("test");
    assert(error == 0);
    error = fileSystem->createFile("test1");
    assert(error == 0);

    error = fileSystem->renameFile("testdgdfgggggggggggg","file");
    assert(error == -2);
    error = fileSystem->renameFile("test","filefdggggggggggggggggggggggggggf");
    assert(error == -2);
    error = fileSystem->renameFile("test","test1");
    assert(error == -4);
    error = fileSystem->renameFile("test2","file");
    assert(error == -3);
    error = fileSystem->renameFile("test","file");
    assert(error == 0);
    delete  fileSystem;
    std::cout << "rename tests passed\n";
}

void copyTest() {
    std::string str1;
    std::string str2;
    int error;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(800);
    assert(error == 0);
    error = fileSystem->createFile("test");
    assert(error == 0);

    error = fileSystem->copy("testtttttttttttttttttttttttttttt","file");
    assert(error == -2);
    error = fileSystem->copy("test","fileeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
    assert(error == -2);
    error = fileSystem->copy("test1","file");
    assert(error == -3);
    error = fileSystem->copy("test","file");
    assert(error == 0);
    error = fileSystem->copy("test","file");
    assert(error == -4);
    error = fileSystem->write("test","ddsgdsgdsgdfddsgdsgdsgdf");
    assert( error == 0 );
    str1 = fileSystem->read("test");
    str2 = fileSystem->read("file");
    assert( str1 != str2);
    error = fileSystem->copy("test","file1");
    assert(error == 0);
    str1 = fileSystem->read("test");
    str2 = fileSystem->read("file1");
    assert( str1 == str2);
    delete fileSystem;
    std::cout << "copy tests passed\n";
}

void deleteTest() {
    int error;
    FileSystem *fileSystem = new FileSystem();
    error = fileSystem->initialize(800);
    assert(error == 0);
    error = fileSystem->createFile("test");
    assert(error == 0);

    error = fileSystem->deleteFile("dfffffffffffffffffffff");
    assert(error == -2);
    error = fileSystem->deleteFile("test1");
    assert(error == -3);
    error = fileSystem->deleteFile("test");
    assert(error == 0);
    error = fileSystem->deleteFile("test");
    assert(error == -3);
    error = fileSystem->createFile("test");
    assert(error == 0);
    error = fileSystem->deleteFile("test");
    assert(error == 0);
    delete fileSystem;
    std::cout << "delete tests passed\n";
}

void run_all_tests() {
    initTest();
    touchTest();
    writeTest();
    readTest();
    renameTest();
    copyTest();
    deleteTest();
    std::cout << "all tests passed\n";
}

int main() {
    run_all_tests();
    FileSystem *fileSystem = new FileSystem();
    fileSystem->open("system", 8000);


    std::string input;
    std::string word;
    int error = 0;


    while (true) {
        std::getline(std::cin, input);
        std::istringstream iss(input, std::istringstream::in);
        std::vector<std::string> wordsVector;
        while (iss >> word) {
            wordsVector.push_back(word);
        }

        if (wordsVector.size()) {
            if (wordsVector[0] == "close") {
                break;
            } else if (wordsVector[0] == "touch") {
                if (wordsVector.size() == 2) {
                    error = fileSystem->createFile(wordsVector[1].c_str());

                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "write") {
                if (wordsVector.size() > 2) {
                    std::string text;
                    for (int i = 2; i < wordsVector.size(); i++) text += wordsVector[i] + " ";
                    error = fileSystem->write(wordsVector[1].c_str(), text.c_str());
                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "read") {
                if (wordsVector.size() == 2) {
                    std::cout << fileSystem->read(wordsVector[1].c_str()) << "\n";

                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "rename") {
                if (wordsVector.size() == 3) {
                    error = fileSystem->renameFile(wordsVector[1].c_str(), wordsVector[2].c_str());

                } else std::cout << "wrong parameters\n";
            } else if (wordsVector[0] == "copy") {
                if (wordsVector.size() == 3) {
                    error = fileSystem->copy(wordsVector[1].c_str(), wordsVector[2].c_str());

                } else std::cout << "wrong parameters\n";
            } else if (wordsVector[0] == "delete") {
                if (wordsVector.size() == 2) {
                    error = fileSystem->deleteFile(wordsVector[1].c_str());

                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "ls") {
                if (wordsVector.size() == 1) {
                    fileSystem->showAllFiles();

                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "dump") {
                if (wordsVector.size() == 1) {
                    fileSystem->dump();

                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "help") {
                if (wordsVector.size() == 1) {
                    std::cout << "commands:\n"
                              << "touch %filename% - create file \n"
                              << "write %filename% %some text% - write to file\n"
                              << "read %filename% - read file\n"
                              << "rename %old_name% %new_name% - rename file\n"
                              << "copy %old_file_name% %new_file_name% - copy file\n"
                              << "delete %filename% - delete file\n"
                              << "ls - show all files\n"
                              << "dump - show file system dump\n"
                              << "cls - clear screen\n"
                              << "close - close file system\n";

                } else std::cout << "wrong parameters\n";

            } else if (wordsVector[0] == "cls") {
                if (wordsVector.size() == 1) {
                    system("CLS");
                } else std::cout << "wrong parameters\n";

            } else std::cout << "command " << wordsVector[0] << " not found\n";


            switch (error) {
                case -1:
                    std::cout << "lack of memory\n";
                    break;
                case -2:
                    std::cout << "wrong file name\n";
                    break;
                case -3:
                    std::cout << "file not found\n";
                    break;
                case -4:
                    std::cout << "file already exist\n";
                    break;
            }

            error = 0;
        }


    }
    return 0;
}