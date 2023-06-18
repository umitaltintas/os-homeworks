#include <iostream>
#include "FileSystem.h"

#include <unordered_map>
#include <functional>


int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " fileSystem.data operation parameters" << std::endl;
        return 1;
    }

    char *dataFileName = argv[1];

    std::string operation = argv[2];

    auto fileSystem = read_filesystem(dataFileName);


    if (operation == "dir") {
        list_dir(fileSystem, argv[3]);
    } else if (operation == "create") {
    } else if (operation == "mkdir") {
        fetch_dir_entry(fileSystem, argv[3]);

    } else if (operation == "rmdir") {

    } else if (operation == "dumpe2fs") {

    } else if (operation == "write") {

    } else if (operation == "read") {

    } else if (operation == "del") {


    }
    return 0;
}
