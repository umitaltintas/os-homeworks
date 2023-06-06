
#ifndef MMU_STATISTICS_H
#define MMU_STATISTICS_H


#include <cstddef>

class MMU_Statistics {
    size_t getAccessCount=0, setAccessCount=0;
    size_t pageMisses=0, pageReplacements=0;
    size_t readCount=0, writeCount=0;

    friend class MMU;


public:
    MMU_Statistics() = default;
    ~MMU_Statistics() = default;



    void printStats();
};


#endif //MMU_STATISTICS_H
