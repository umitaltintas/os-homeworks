#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "page_table_entry.h"
#include <vector>

#define TOTAL_FRAMES 32

struct Frame
{
    int data[4096]; // assuming frame size of 4096 integers
};

class MemoryManager
{
public:
    MemoryManager();

    int get(int virtualAddress);
    void set(int virtualAddress, int value);

private:
    std::vector<Frame> physicalMemory;
    std::vector<PageTableEntry> pageTable;
};

#endif // MEMORY_MANAGER_H
