#include <iostream>
#include <unistd.h> // for void *sbrk(intptr_t __delta) noexcept(true)
#include <cassert>

struct MemoryBlock
{
    size_t size;
    bool isUsed { false };
    MemoryBlock *next;
    uint64_t data[1];
};

static MemoryBlock *heapStart = nullptr;
static MemoryBlock *head = heapStart;

size_t align(size_t n)
{   
    if(n % sizeof(uint64_t) == 0)
        return n;
    else 
        return (n + (((n / sizeof(uint64_t)) + 1) * sizeof(uint64_t) - n));
}

size_t alloc_size(size_t size)
{
    // - sizeof(MemoryBlock::data) because we count it twise
    return (size + sizeof(MemoryBlock) - sizeof(MemoryBlock::data));
}

MemoryBlock* request_block(size_t size)
{
    // By calling the sbrk(0), we obtain the pointer to the current heap break — this is the beginning position of the newly allocated block.
    MemoryBlock *block = (MemoryBlock *)sbrk(0);

    // If sbrk() returns (void*)-1, it means that the memory allocation request failed (Out Of Memory) OOM
    if(sbrk(alloc_size(size)) == (void *)-1)
        return nullptr;

    return block;
}

// allocates a block of size AT-LEAST size bytes
uint64_t* allocate(size_t size)
{
    size = align(size);

    MemoryBlock *block = request_block(size);
    block->size = size;
    block->isUsed = true;

    if(heapStart == nullptr)
        heapStart = block;

    if(head != nullptr)
        head->next = block;

    head = block;

    return block->data;
}

void free(uint64_t *data)
{
    MemoryBlock* block = get_header(data);

    block->isUsed = false;
}
// convert a pointer to a memory block's data section into a pointer to the memory block itself
MemoryBlock* get_header(uint64_t *data) {
    return (MemoryBlock *)((char *)data + sizeof(MemoryBlock::data) - sizeof(MemoryBlock));
}

int main() {
    

    return 0;
}