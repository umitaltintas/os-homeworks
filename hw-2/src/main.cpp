
#include <iostream>
#include <thread>
#include <future>
#include "MMU.h"
#include "MMU_Config.h"

#define ARRAY_SIZE 1000000

using namespace std;


void printUsage() {

    std::cout << "Usage: " << " <frame size> <physical frames> <virtual frames> <algorithm> <table type> <disk file>\n"
              << std::endl;

    exit(EXIT_FAILURE);
}

MMU_Config argsParser(int argc, char *argv[]) {
    if (argc != 8) {
        printUsage();
        std::cout << argc << std::endl;
        exit(EXIT_FAILURE);
    }
    int frameSize = std::stoi(argv[1]);
    int physicalFrames = std::stoi(argv[2]);
    int virtualFrames = std::stoi(argv[3]);
    std::string algorithm = argv[4];
    std::string tableType = argv[5];
    int printThreshold = std::stoi(argv[6]);
    std::string diskFile = argv[7];
    if (frameSize < 2 || physicalFrames < 2 || virtualFrames < 2) {
        printUsage();
    }
    MMU_Config::ReplacementAlgorithm algo;
    if (std::equal(algorithm.begin(), algorithm.end(), "WSC")) {
        algo = MMU_Config::WSC;
    } else if (std::equal(algorithm.begin(), algorithm.end(), "SC")) {
        algo = MMU_Config::SC;
    } else { algo = MMU_Config::LRU; }

    MMU_Config config = MMU_Config::Builder().setFrameSize(frameSize).setNumPhysical(physicalFrames).setNumVirtual(
                    virtualFrames).setPrintThreshold(printThreshold)
            .setAlgo(algo).setPath(diskFile).build();

    return config;
}

void
matrixVectorMultiply(MMU &mmu, int startMatrix, int matrixRows, int matrixColumns, int startVector, int startResult) {
    // Assuming 'vector' is the same length as the number of columns in the matrix
    for (int i = 0; i < matrixRows; ++i) {
        // Initialize the result element in the MMU to 0
        mmu.set(startResult + i, 0);

        for (int j = 0; j < matrixColumns; ++j) {
            // Retrieve element (i, j) from the 2D matrix in the MMU
            int matrixElement = mmu.get(startMatrix + i * matrixColumns + j);

            // Retrieve vector element from the MMU
            int vectorElement = mmu.get(startVector + j);

            // Multiply matrix element by vector element and add to the result
            int resultElement = mmu.get(startResult + i);
            mmu.set(startResult + i, resultElement + matrixElement * vectorElement);
        }
    }
}


int vectorVectorMultiply(MMU &mmu, uint32_t vec1Start, uint32_t vec2Start, uint32_t length) {
    int result = 0;
    for (uint32_t i = 0; i < length; ++i) {
        result += mmu.get(vec1Start + i) * mmu.get(vec2Start + i);
    }
    return result;
}


void arraySummation(MMU &mmu, uint32_t first_array_index, int addition, uint32_t length
) {

    for (uint32_t i = 0; i < length; ++i) {
        mmu.set(first_array_index + i, mmu.get(first_array_index + i) + addition);
    }
}

int linearSearch(MMU &mmu, uint32_t start, uint32_t end, int target) {
    for (uint32_t i = start; i < end; ++i) {
        if (mmu.get(i) == target) {
            return i;
        }
    }
}

int binarySearch(MMU &mmu, uint32_t start, uint32_t end, int target) {
    while (start <= end) {
        uint32_t mid = start + (end - start) / 2;
        int midValue = mmu.get(mid);

        if (midValue == target) {
            return mid;
        } else if (midValue < target) {
            start = mid + 1;
        } else {
            end = mid - 1;
        }
    }
}

void fillMMU(MMU &mmu, uint32_t start, uint32_t end) {
    srand(time(NULL));
    for (uint32_t i = start; i < end; ++i) {
        mmu.set(i, rand() % 100);  // Random integers in the range 0-99
    }
}


int main(int argc, char *argv[]) {

    MMU_Config config = argsParser(argc, argv);
    MMU mmu(config);

    fillMMU(mmu, 0, ARRAY_SIZE);

    // Create 1 thread for the matrix-vector multiplication

    std::thread th1(matrixVectorMultiply, std::ref(mmu), 0, 200, 200, 60000, 70000);


    // Create 1 thread for the vector-vector multiplication
    auto feature = std::async(vectorVectorMultiply, std::ref(mmu), 80000, 80200, 200);

    int vectorResult = feature.get();


    // Join these threads before proceeding
    th1.join();


    std::thread th3(arraySummation, std::ref(mmu), 70000, vectorResult, 200);


    // Wait for the summation to finish
    th3.join();

    // Perform searches
    std::vector<int> searchTargets = {mmu.get(70022), 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<std::thread> searchThreads;
    for (int target: searchTargets) {
        // Use either linearSearch or binarySearch depending on your needs
        //use feature to get the result of the search
        std::thread th(linearSearch, std::ref(mmu), 70000, 70200, target);
        searchThreads.push_back(std::move(th));


    }

    //sort the array before performing binary search
    //use bubble sort
    for (int i = 70000; i < 70200; ++i) {
        for (int j = 70000; j < 70200 - i - 1; ++j) {
            if (mmu.get(j) > mmu.get(j + 1)) {
                int temp = mmu.get(j);
                mmu.set(j, mmu.get(j + 1));
                mmu.set(j + 1, temp);
            }
        }
    }


    for (int target: searchTargets) {
        // Use either linearSearch or binarySearch depending on your needs
        //use feature to get the result of the search
        std::thread th(binarySearch, std::ref(mmu), 70000, 70200, target);
        searchThreads.push_back(std::move(th));
    }

    // Wait for all search threads to finish
    for (std::thread &th: searchThreads) {
        th.join();
    }


    mmu.printStats();
    return 0;
}