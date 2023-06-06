
#ifndef HW_2_MMU_H
#define HW_2_MMU_H

#include <list>
#include <vector>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <thread>
#include <iostream>

#include "MMU_Config.h"
#include "MMU_Statistics.h"

using std::list;
using std::vector;
#define GET_BIT(a, n) (((a) >> (13 - (n))) & 1)
#define SET_BIT(a, n) ((a) |= 1 << (13 - (n)))
#define CLR_BIT(a, n) ((a) &= ~(1 << (13 - (n))))

#define TIME_THRESHOLD 10000

class MMU {

    struct PageTableEntry {
        uint32_t isEntryPresent: 1;
        uint32_t isEntryModified: 1;
        uint32_t referenceCounter: 14;
        uint32_t physicalAddress: 16;

        // Constructor
        PageTableEntry() : isEntryPresent(0), isEntryModified(0), referenceCounter(0), physicalAddress(0) {}
    };


    struct PageTable {
        uint32_t virtualAddressSpaceSize;
        uint32_t physicalMemorySize;
        uint32_t pageTableSize;
        std::vector<PageTableEntry *> pageTableEntries;
        std::chrono::time_point<std::chrono::system_clock> lastAccessTime;
    };


public:
    int get(unsigned int index);

    void set(unsigned int index, int value);

    MMU(const MMU_Config &cfg);

    void printStats();

    ~MMU();





private:
    PageTable *table;
    MMU_Config config;
    MMU_Statistics stats;
    int *physicalMemory;
    std::mutex mtx;
    int virtualMemoryFileDescriptor;

    vector<uint32_t> wscLst;


    std::function<uint32_t()> algo;

    void updateAccessTime();

    void initPageTable();

    void initVirtualMemory();

    void initPhysicalMemory();


    uint32_t getPhysicalFrameFromVirtual(uint32_t virtualFrame);

    uint32_t physicalAddress(unsigned int index, uint8_t mod);

    uint32_t lruAlgo();


    uint32_t scAlgo();

    uint32_t wscAlgo();

    int64_t findEmptyMemoryFrame();

    uint32_t handlePageFaultForVirtualFrame(uint32_t virtualFrame);

    void setPage(uint32_t vFrame, uint32_t pFrame);

    void isTimeThresholdReached();

    bool isMemoryFull();

    bool isFrameOccupied(uint32_t frameIndex);

    void init();
};


#endif //HW_2_MMU_H
