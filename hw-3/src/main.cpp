#include "memory_manager.h"
#include <iostream>
#include <iostream>

int main()
{
    MemoryManager memManager;

    memManager.set(0, 12345);
    memManager.set(5000, 12345);

    int value0 = memManager.get(0);
    int value5000 = memManager.get(5000);

    std::cout << "Value at virtual address 0: " << value0 << std::endl;
    std::cout << "Value at virtual address 5000: " << value5000 << std::endl;

    return 0;
}
