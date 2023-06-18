#include <iostream>
#include "FileSystem.h"




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
    } else if (operation == "mkdir") {
        fetch_dir_entry(fileSystem, argv[3]);
    } else if (operation == "rmdir") {
        delete_dir(fileSystem, argv[3]);
    } else if (operation == "dumpe2fs") {

    } else if (operation == "write") {
        char *sourceFileName = argv[4];
        FILE *linuxFile = fopen(sourceFileName, "r");
        if (linuxFile == nullptr) {
            std::cerr << "Error: " << std::string(sourceFileName) << " not found" << std::endl;
            return 1;
        }

        char *content = nullptr;
        size_t size = 0;
        fseek(linuxFile, 0, SEEK_END);
        size = ftell(linuxFile);
        fseek(linuxFile, 0, SEEK_SET);
        content = (char *) malloc(size);
        fread(content, 1, size, linuxFile);
        fclose(linuxFile);

        write_file(fileSystem, argv[3], content, size);


    } else if (operation == "read") {
        read_file(fileSystem, argv[3], argv[4]);
    } else if (operation == "del") {
        delete_file(fileSystem, argv[3]);

    }
    else if(operation=="dump2fs"){
        dumpe2fs(fileSystem);
    }


    return 0;
}
