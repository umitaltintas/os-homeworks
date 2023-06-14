#include <iostream>
#include "FileSystem.h"

#include <unordered_map>
#include <functional>

// Define a type alias for a function pointer that takes two string arguments
using OperFunction = std::function<void(FileSystem &, const std::string &, const std::string &)>;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " fileSystem.data operation parameters" << std::endl;
        return 1;
    }

    std::string dataFileName = argv[1];
    FileSystem fileSystem(dataFileName);
    std::string operation = argv[2];
    if (operation == "dir") {
        fileSystem.dir(argv[3]);
    } else if (operation == "mkdir") {
        fileSystem.mkdir(argv[3]);
    } else if (operation == "rmdir") {
        fileSystem.rmdir(argv[3]);
    } else if (operation == "dumpe2fs") {
        fileSystem.dumpe2fs();
    } else if (operation == "write") {
        fileSystem.write(argv[3], argv[4]);
    } else if (operation == "read") {
        fileSystem.read(argv[3], argv[4]);
    } else if (operation == "del") {
        fileSystem.del(argv[3]);
    }
    return 0;
}
