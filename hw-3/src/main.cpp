#include "memory_manager.h"
#include <iostream>
#include <iostream>
#include <cmath>

int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        std::cout << "Usage: operateArrays <frame size> <physical frames> <virtual frames> <algorithm> <table type> <disk file>\n";
        std::cout << argc << std::endl;
        return 1;
    }

    // Parse command line arguments
    int frameSize = std::pow(2, std::stoi(argv[1]));
    int numPhysicalFrames = std::pow(2, std::stoi(argv[2]));
    int numVirtualFrames = std::pow(2, std::stoi(argv[3]));
    std::string algorithm = argv[4];
    std::string tableType = argv[5];
    std::string diskFile = argv[6];

    // Pass these to your MemoryManager or appropriate object
    MemoryManager memManager(frameSize, numPhysicalFrames, numVirtualFrames, algorithm, tableType, diskFile);

    // Proceed with your simulation here...
}
