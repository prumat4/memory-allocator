#include "../src/alloc.hpp"

extern MemoryBlock *heapStart;
extern MemoryBlock *head;
extern MemoryBlock *searchStart;

extern SearchMode searchMode;

extern std::list<MemoryBlock* > freeList;

int main() 
{
    int *ptr = (int*)allocate(64);

    std::cout << ptr << std::endl;
    std::cout << *ptr << std::endl;

    *ptr = 666;

    std::cout << ptr << std::endl;
    std::cout << *ptr << std::endl;

    return 0;
}