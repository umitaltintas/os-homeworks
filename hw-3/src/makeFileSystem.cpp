
#include <iostream>
#include <thread>
#include <future>
#include "FileSystem.h"
int main(int argc, char** argv) {
    // Ensure the correct number of command-line arguments were provided.
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <block size in KB> <filename>\n";
        return 1;
    }

    // Parse the block size.
    int blockSizeKB = std::stoi(argv[1]);
    if (blockSizeKB <= 0 || blockSizeKB > 16) {
        std::cerr << "Invalid block size. Must be between 1 and 16 (inclusive).\n";
        return 1;
    }

    // Get the filename.
    std::string filename = argv[2];

    // Create the file system.
    FileSystem fileSystem(filename, blockSizeKB);
    fileSystem.create();

    return 0;
}