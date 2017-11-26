
#include <iostream>
#include <vector>
#include "FileSystem.h"


int main() {

    FileSystem *fileSystem = new FileSystem();
    fileSystem->initialize(8000);
//    fileSystem->createFile("mama");
//    fileSystem->createFile("papa");
//    fileSystem->createFile("as");
    //   fileSystem->write("mama", "1234567890qweqweqweqevvfvfvfvffvfvfvfvfvfv");
    //   std::cout << fileSystem->read("mama") << std::endl;
//    fileSystem->showAllFiles();
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