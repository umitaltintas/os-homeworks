#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "page_table_entry.h"
#include <vector>
#include <string>
#include <fstream>

#define TOTAL_FRAMES 32

struct Frame
{
    int data[4096]; // assuming frame size of 4096 integers
};

class MemoryManager
{
public:
    MemoryManager(int frameSize, int numPhysicalFrames, int numVirtualFrames, std::string algorithm, std::string tableType, std::string diskFile);

    int get(int virtualAddress);
    void set(int virtualAddress, int value);

private:
    int frameSize;
    int numPhysicalFrames;
    int numVirtualFrames;
    std::string algorithm;
    std::string tableType;
    std::vector<Frame> physicalMemory;
    std::vector<PageTableEntry> pageTable;
    std::fstream backingStore; // Disk for the backing store

    void handlePageFault(int pageNumber);
    void evictPage();
    void writeToBackingStore(int pageNumber);
    void readFromBackingStore(int pageNumber);
};

#endif // MEMORY_MANAGER_H
