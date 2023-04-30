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
// convert a pointer to a memory block's data section into a pointer to the memory block itself
MemoryBlock* get_header(uint64_t *data) {
    return (MemoryBlock *)((char *)data + sizeof(MemoryBlock::data) - sizeof(MemoryBlock));
}

void free(uint64_t *data)
{
    MemoryBlock* block = get_header(data);
    block->isUsed = false;
}

MemoryBlock* first_fit_search(size_t size)
{
    MemoryBlock *block = heapStart;

    while(block != nullptr)
    {
        if(block->isUsed || block->size < size)
        {
            block = block->next;
            continue;
        }

        return block;
    }

    return nullptr;
}

MemoryBlock* find_block(size_t size)
{
    return first_fit_search(size);
}
// allocates a block of size AT-LEAST size bytes
uint64_t* allocate(size_t size)
{
    size = align(size);
    
    // if it is possible to reuse block than we do it 
    if(MemoryBlock *block = find_block(size))
        return block->data;
    
    // else request new block from OS
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

int main() {
        // --------------------------------------
    // Test case 1: Alignment
    //
    // A request for 3 bytes is aligned to 8.
    //
    
    auto p1 = allocate(3);                        // (1)
    auto p1b = get_header(p1);
    assert(p1b->size == sizeof(uint64_t));
    
    // --------------------------------------
    // Test case 2: Exact amount of aligned bytes
    //
    
    auto p2 = allocate(8);                        // (2)
    auto p2b = get_header(p2);
    assert(p2b->size == 8);
 
    free(p2);
    assert(p2b->isUsed == false);
    auto p3 = allocate(8);
    auto p3b = get_header(p3);
    assert(p3b->size == 8);
    assert(p3b == p2b);
    
    puts("\nAll assertions passed!\n");
    return 0;
}