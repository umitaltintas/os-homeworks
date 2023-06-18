
#include <iostream>
#include <thread>
#include <future>
#include "FileSystem.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <block size in KB> <filename>\n";
        return 1;
    }

    int blockSizeKB = std::stoi(argv[1]);
    if (blockSizeKB <= 0 || blockSizeKB > 16) {
        std::cerr << "Invalid block size. Must be between 1 and 16 (inclusive).\n";
        return 1;
    }

    char *filename = argv[2];

    create_filesystem(filename, blockSizeKB * 1024);

    return 0;
}