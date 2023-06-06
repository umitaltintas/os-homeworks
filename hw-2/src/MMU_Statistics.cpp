
#include <iostream>
#include "MMU_Statistics.h"

void MMU_Statistics::printStats() {

    std::cout << "getAccessCount: " << getAccessCount << std::endl;
    std::cout << "setAccessCount: " << setAccessCount << std::endl;
    std::cout << "pageMisses: " << pageMisses << std::endl;
    std::cout << "pageReplacements: " << pageReplacements << std::endl;
    std::cout << "readCount: " << readCount << std::endl;
    std::cout << "writeCount: " << writeCount << std::endl;

}
