
#include "MMU.h"
#include <thread>

MMU::~MMU() {
    close(virtualMemoryFileDescriptor);
    delete[] physicalMemory;
    for (uint32_t i = 0; i < table->virtualAddressSpaceSize; ++i) {
        table->pageTableEntries.clear();
    }

    this->wscLst.clear();

    delete[] table->pageTableEntries.data();
}

MMU::MMU(const MMU_Config &cfg) : config(cfg) {

    stats = MMU_Statistics();
    init();

    switch (config.algo) {

        case MMU_Config::ReplacementAlgorithm::SC:
            algo = [this]() { return this->scAlgo(); };
            break;
        case MMU_Config::ReplacementAlgorithm::LRU:
            algo = [this]() { return this->lruAlgo(); };
            break;
        case MMU_Config::ReplacementAlgorithm::WSC:
            algo = [this]() { return this->wscAlgo(); };
            break;
        default:
            throw std::runtime_error("Invalid algorithm");
    }

}

void MMU::init() {
    initPageTable();
    initVirtualMemory();
    initPhysicalMemory();
}

int MMU::get(unsigned int index) {
    std::lock_guard<std::mutex> lock(mtx);
    // if threshold is reached, print stats
    if ((stats.getAccessCount + stats.setAccessCount )% config.printThreshold == 0) {
        std::cout << "Printing stats" << std::endl;
        printStats();
    }
    uint32_t addr = physicalAddress(index, 0);
    int res = physicalMemory[addr];
    return res;
}

void MMU::set(unsigned int index, int value) {
    std::lock_guard<std::mutex> lock(mtx);
    if ((stats.getAccessCount + stats.setAccessCount) % (config.printThreshold) == 0) {
        std::cout << "Printing stats" << std::endl;
        printStats();
        std::cout << std::endl;
    }
    uint32_t addr = physicalAddress(index, 1);
    physicalMemory[addr] = value;
}

uint32_t MMU::getPhysicalFrameFromVirtual(uint32_t virtualFrame) {


    if (0 != table->pageTableEntries[virtualFrame]->isEntryPresent) {
        return table->pageTableEntries[virtualFrame]->physicalAddress;
    }
    stats.pageMisses++;
    int64_t pFrame = findEmptyMemoryFrame();

    if (-1 == pFrame) {
        stats.pageReplacements++;
        pFrame = handlePageFaultForVirtualFrame(virtualFrame);
    }
    setPage(virtualFrame, pFrame);
    return pFrame;
}

int64_t MMU::findEmptyMemoryFrame() {
    if (isMemoryFull()) {
        return -1;
    }

    // Find first empty frame
    for (uint32_t frameIndex = 0; frameIndex < table->physicalMemorySize; frameIndex++) {
        if (!isFrameOccupied(frameIndex)) {
            return frameIndex;
        }
    }

    return -1;  // Should not reach here due to isMemoryFull() check

}

bool MMU::isMemoryFull() {
    // Check each frame
    for (uint32_t frameIndex = 0; frameIndex < table->physicalMemorySize; frameIndex++) {
        if (!isFrameOccupied(frameIndex)) {
            return false;
        }
    }

    // If we reach this point, memory is full
    return true;
}

bool MMU::isFrameOccupied(uint32_t frameIndex) {
    // Check each page table entry
    for (uint32_t entryIndex = 0; entryIndex < table->virtualAddressSpaceSize; entryIndex++) {
        if (table->pageTableEntries[entryIndex]->isEntryPresent &&
            table->pageTableEntries[entryIndex]->physicalAddress == frameIndex) {
            return true;
        }
    }
    return false;
}


uint32_t MMU::handlePageFaultForVirtualFrame(uint32_t virtualFrame) {
    uint32_t victimVirtualFrame = algo();
    PageTableEntry *entry = table->pageTableEntries[victimVirtualFrame];
    uint32_t physicalAddress = entry->physicalAddress;
    ssize_t size = sizeof(int) * table->pageTableSize;
    off_t location = size * victimVirtualFrame;
    int *ptr = physicalMemory;
    ptr += table->pageTableSize * physicalAddress;
    if (0 != entry->isEntryModified) {
        if (size != pwrite(virtualMemoryFileDescriptor, ptr, size, location)) {
            std::exit(EXIT_FAILURE);
        }
        stats.writeCount++;
    }
    entry->isEntryPresent = 0;
    entry->isEntryModified = 0;
    entry->referenceCounter = 0;
    return physicalAddress;
}

void MMU::setPage(uint32_t vFrame, uint32_t pFrame) {
    ssize_t size = sizeof(int) * table->pageTableSize;
    off_t loc = size * vFrame;
    int *ptr = physicalMemory;
    ptr += table->pageTableSize * pFrame;
    if (size != pread(virtualMemoryFileDescriptor, ptr, size, loc)) {
        std::cerr << "setPage, pread" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    PageTableEntry *entry = table->pageTableEntries[vFrame];
    entry->isEntryPresent = 1;
    entry->isEntryModified = 0;
    entry->referenceCounter = 0;
    entry->physicalAddress = pFrame;
    stats.readCount++;
    if (MMU_Config::WSC == config.algo) {
        if (wscLst.size() < 1 + table->physicalMemorySize) {
            wscLst.push_back(vFrame);
        } else if (1 == (wscLst)[0]) {
            (wscLst)[wscLst.size() - 1] = vFrame;
        } else {
            (wscLst)[(wscLst)[0] - 1] = vFrame;
        }
    } else if (MMU_Config::LRU == config.algo) {
        wscLst.push_back(vFrame);
    }
}


void MMU::isTimeThresholdReached() {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point lastClock = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            table->lastAccessTime);

    std::chrono::duration<double, std::micro> time_diff = now - lastClock;


    if (time_diff.count() < TIME_THRESHOLD) {
        return;
    }

    for (uint32_t i = 0; i < table->virtualAddressSpaceSize; ++i) {
        if (0 == table->pageTableEntries[i]->isEntryPresent) continue;
        table->pageTableEntries[i]->referenceCounter >>= 1;
    }
    table->lastAccessTime = now;
}

void MMU::initPageTable() {
    table = new PageTable;
    memset(table, 0, sizeof(PageTable));
    table->virtualAddressSpaceSize = pow(2, config.numVirtual);
    table->physicalMemorySize = pow(2, config.numPhysical);
    table->pageTableSize = pow(2, config.frameSize);


    table->pageTableEntries.resize(table->virtualAddressSpaceSize);

    for (uint32_t i = 0; i < table->virtualAddressSpaceSize; ++i) {
        table->pageTableEntries[i] = new PageTableEntry;
    }

    updateAccessTime();


}

void MMU::initVirtualMemory() {
    virtualMemoryFileDescriptor = open(config.path.c_str(), O_RDWR | O_CREAT | O_SYNC, 0644);
    if (-1 == virtualMemoryFileDescriptor) {
        throw std::runtime_error("initVirtualMemory, open");
    }
    if (-1 == ftruncate(virtualMemoryFileDescriptor, 0)) {
        throw std::runtime_error("initVirtualMemory, ftruncate");
    }
    if (-1 ==
        ftruncate(virtualMemoryFileDescriptor, sizeof(int) * table->pageTableSize * table->virtualAddressSpaceSize)) {
        throw std::runtime_error("initVirtualMemory, ftruncate");
    }

    updateAccessTime();
}

void MMU::initPhysicalMemory() {
    physicalMemory = new int[table->physicalMemorySize * table->pageTableSize];
    memset(physicalMemory, 0, sizeof(int) * table->physicalMemorySize * table->pageTableSize);
    updateAccessTime();
}

uint32_t MMU::physicalAddress(unsigned int index, uint8_t mod) {
    uint32_t vFrame = index >> config.frameSize;
    if (vFrame >= table->virtualAddressSpaceSize) {
        std::cerr << "physicalAddress, vFrame, index(" << vFrame << ")\n";
        std::exit(EXIT_FAILURE);
    }
    uint32_t addr = index - (vFrame << config.frameSize);
    uint32_t pFrame = getPhysicalFrameFromVirtual(vFrame);

    switch (config.algo) {
        case MMU_Config::ReplacementAlgorithm::LRU:
        case MMU_Config::ReplacementAlgorithm::WSC:
            isTimeThresholdReached();
            break;
        case MMU_Config::ReplacementAlgorithm::SC:
            wscLst.push_back(vFrame);
            break;
        default:
            std::cerr << "physicalAddress, switch\n";
            std::exit(EXIT_FAILURE);
    }
    SET_BIT(table->pageTableEntries[vFrame]->referenceCounter, 0);
    if (mod) {
        table->pageTableEntries[vFrame]->isEntryModified = 1;
        stats.setAccessCount++;
    } else {
        stats.getAccessCount++;
    }
    addr += (pFrame << config.frameSize);

    return addr;
}

void MMU::updateAccessTime() {
    table->lastAccessTime = std::chrono::system_clock::now();
}


uint32_t MMU::lruAlgo() {
    PageTableEntry *entry;
    uint32_t selectedPage;
    uint16_t referenceCounter, isEntryModified, lowestPriority = /* max uint16_t*/ 65535, replacementPriority;

    for (uint32_t i = 0; i < table->virtualAddressSpaceSize; ++i) {
        entry = table->pageTableEntries[i];
        if (0 == entry->isEntryPresent) {
            continue;
        }
        referenceCounter = entry->referenceCounter;
        isEntryModified = entry->isEntryModified;
        replacementPriority = isEntryModified | (referenceCounter << 1);
        if (replacementPriority < lowestPriority) {
            lowestPriority = replacementPriority;
            selectedPage = i;
            if (0 == replacementPriority) {
                break;
            }
        }
    }
    return selectedPage;
}


uint32_t MMU::wscAlgo() {
    PageTableEntry *entry;
    uint32_t victim, pagePosition;
    uint32_t currentPosition = wscLst.at(0);

    while (true) {
        pagePosition = wscLst.at(currentPosition);
        entry = table->pageTableEntries[pagePosition];
        currentPosition++;
        if (wscLst.size() <= currentPosition) {
            currentPosition = 1;
        }

        if (GET_BIT(entry->referenceCounter, 0) == 0) {
            victim = pagePosition;
            break;
        }
        CLR_BIT(entry->referenceCounter, 0);
    }
    (wscLst)[0] = currentPosition;
    return victim;
}


uint32_t MMU::scAlgo() {
    uint32_t selected = wscLst.front();
    // pop fornt
    wscLst.erase(wscLst.begin());

    return selected;
}

void MMU::printStats() {
    stats.printStats();
}





