#include "memory_manager.h"

#include <mutex>

std::mutex mtx;

MemoryManager::MemoryManager() : physicalMemory(TOTAL_FRAMES), pageTable(1024)
{
}

int MemoryManager::get(int virtualAddress)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pageNumber = virtualAddress / 4096;
    int offset = virtualAddress % 4096;

    if (pageTable[pageNumber].isValid())
    {
        pageTable[pageNumber].updateTimestamp();
        return physicalMemory[pageTable[pageNumber].getFrameNumber()].data[offset];
    }
    else
    {
        // Page fault handling
        // Call your page replacement algorithm here and replace the required page
        // After page replacement, retry accessing the data
        return -1; // placeholder return statement for now
    }
}

void MemoryManager::set(int virtualAddress, int value)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pageNumber = virtualAddress / 4096;
    int offset = virtualAddress % 4096;

    if (pageTable[pageNumber].isValid())
    {
        pageTable[pageNumber].updateTimestamp();
        physicalMemory[pageTable[pageNumber].getFrameNumber()].data[offset] = value;
        pageTable[pageNumber].markDirty();
    }
    else
    {
        // Page fault handling
        // Call your page replacement algorithm here and replace the required page
        // After page replacement, retry accessing the data
    }
}
