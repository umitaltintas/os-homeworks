#include "memory_manager.h"
#include <mutex>

std::mutex mtx;

MemoryManager::MemoryManager(int frameSize, int numPhysicalFrames, int numVirtualFrames, std::string algorithm, std::string tableType, std::string diskFile)
    : physicalMemory(numPhysicalFrames), pageTable(numVirtualFrames), frameSize(frameSize), numPhysicalFrames(numPhysicalFrames), numVirtualFrames(numVirtualFrames), algorithm(algorithm), tableType(tableType)
{
    backingStore.open(diskFile, std::ios::in | std::ios::out | std::ios::binary);
    if (!backingStore)
    {
        backingStore.open(diskFile, std::ios::out | std::ios::binary);
        // Consider initializing the file to a known state here, if necessary.
    }
    backingStore.close();
    backingStore.open(diskFile, std::ios::in | std::ios::out | std::ios::binary);
}

int MemoryManager::get(int virtualAddress)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pageNumber = virtualAddress / frameSize;
    int offset = virtualAddress % frameSize;

    for (auto &entry : pageTable)
    {
        if (entry.getVirtualPageNumber() == pageNumber && entry.isValid())
        {
            entry.updateTimestamp();
            return physicalMemory[entry.getFrameNumber()].data[offset];
        }
    }

    handlePageFault(pageNumber);

    return -1; // placeholder return statement for now
}

void MemoryManager::set(int virtualAddress, int value)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pageNumber = virtualAddress / frameSize;
    int offset = virtualAddress % frameSize;

    for (auto &entry : pageTable)
    {
        if (entry.getVirtualPageNumber() == pageNumber && entry.isValid())
        {
            entry.updateTimestamp();
            physicalMemory[entry.getFrameNumber()].data[offset] = value;
            entry.markDirty();
            return;
        }
    }

    handlePageFault(pageNumber);
}

void MemoryManager::handlePageFault(int pageNumber)
{
    evictPage(); // Placeholder for page eviction logic

    // Read the new page from the backing store into the frame
    int frameNumber /* get the frame number for the new page */;
    backingStore.seekg(pageNumber * frameSize * sizeof(int));                                               // Position the file pointer to the start of the page
    backingStore.read(reinterpret_cast<char *>(physicalMemory[frameNumber].data), frameSize * sizeof(int)); // Read the page into the frame

    // Update the page table entry for the new page
    pageTable[pageNumber].setFrameNumber(frameNumber);
    pageTable[pageNumber].setVirtualPageNumber(pageNumber);
    pageTable[pageNumber].markValid();
    pageTable[pageNumber].markDirty();
    pageTable[pageNumber].updateTimestamp();
    pageTable[pageNumber].updateLastUsed();
}

void MemoryManager::evictPage()
{
    int victimPage = -1;

    // Determine the victim page based on the specified page replacement algorithm
    if (algorithm == "LRU")
    {
        // Implement LRU eviction logic
        // Find the page with the least recently used timestamp
        // Set victimPage to the index of the page in the page table
    }
    else if (algorithm == "FIFO")
    {
        // Implement FIFO eviction logic
        // Select the page based on the order of arrival
        // Set victimPage to the index of the page in the page table
    }
    else if (algorithm == "Custom")
    {
        // Implement your custom page replacement algorithm
        // Set victimPage to the index of the page in the page table
    }
    else
    {
        // Invalid algorithm specified
        // Handle the error appropriately
        return;
    }

    if (victimPage == -1)
    {
        // No victim page found, handle the error appropriately
        return;
    }

    // If the victim page is dirty, write it back to the backing store
    if (pageTable[victimPage].isDirty())
    {
        writeToBackingStore(victimPage);
    }

    // Reset the page table entry for the victim page
    pageTable[victimPage].markInvalid();
    pageTable[victimPage].markClean();
    pageTable[victimPage].setFrameNumber(-1);
    pageTable[victimPage].updateTimestamp();
    pageTable[victimPage].updateLastUsed();
}
void MemoryManager::writeToBackingStore(int pageNumber)
{
    backingStore.seekp(pageNumber * frameSize * sizeof(int));                                                                                 // Position the file pointer to the start of the page
    backingStore.write(reinterpret_cast<const char *>(physicalMemory[pageTable[pageNumber].getFrameNumber()].data), frameSize * sizeof(int)); // Write the page data to the backing store
    // Check for any write errors if necessary
}
