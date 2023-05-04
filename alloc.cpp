#include <iostream>
#include <unistd.h>
#include <cassert>  

#define SIZE_T_MAX 65535

struct MemoryBlock
{
    size_t size;
    bool isUsed { false };
    MemoryBlock *next;
    uint64_t data[1];
};

static MemoryBlock *heapStart = nullptr;
static MemoryBlock *head = heapStart;
static MemoryBlock *searchStart = heapStart;

enum class SearchMode
{
    FirstFit,
    NextFit,
    BestFit
};

static auto searchMode = SearchMode::FirstFit;

void resetHeap() {
    if (heapStart == nullptr) {
        return;
    }
    
    brk(heapStart);
    
    heapStart = nullptr;
    head = nullptr;
    searchStart = nullptr;
}

void init(SearchMode mode)
{
    searchMode = mode;
    resetHeap();
}

size_t align(size_t n)
{   
    if(n % sizeof(uint64_t) == 0)
        return n;
    else 
        return (n + (((n / sizeof(uint64_t)) + 1) * sizeof(uint64_t) - n));
}

size_t alloc_size(size_t size)
{
    return (size + sizeof(MemoryBlock) - sizeof(MemoryBlock::data));
}

MemoryBlock* request_block(size_t size)
{
    MemoryBlock *block = (MemoryBlock *)sbrk(0);

    if(sbrk(alloc_size(size)) == (void *)-1)
        return nullptr;

    return block;
}

MemoryBlock* get_header(uint64_t *data) 
{
    return (MemoryBlock *)((char *)data + sizeof(MemoryBlock::data) - sizeof(MemoryBlock));
}

void free(uint64_t *data)
{
    MemoryBlock* block = get_header(data);
    block->isUsed = false;
}

bool can_split(MemoryBlock *block, size_t size)
{
    return block->size > size;
}

MemoryBlock* split(MemoryBlock* block, size_t size)
{
    MemoryBlock *newBlock = (MemoryBlock*)(char *)block + size;
    newBlock->size = block->size - size;
    newBlock->isUsed = false;
    newBlock->next = block->next;

    block->size = size;
    block->isUsed = true;
    block->next = newBlock;

    return block;
}

MemoryBlock* list_allocate(MemoryBlock *block, size_t size)
{
    if(can_split(block, size))
        block = split(block, size);
    
    block->isUsed = true;
    block->size = size;

    return block;
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

MemoryBlock* next_fit_search(size_t size)
{
    MemoryBlock *block = searchStart;

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

MemoryBlock* best_fit_search(size_t size)
{
    MemoryBlock *block = heapStart;
    size_t sizeDifference = SIZE_T_MAX;

    while(block != nullptr)
    {
        if(!(block->isUsed) && block->size >= size)
        {
            if(sizeDifference > block->size - size)
                sizeDifference = block->size - size;
        }

        block = block->next;        
    }

    if(sizeDifference == SIZE_T_MAX)
        return nullptr;

    block = heapStart;
    while(block != nullptr)
    {
        if(!(block->isUsed) && block->size >= size)
        {
            if(sizeDifference == block->size - size)
                break;
        }

        block = block->next;        
    }

    list_allocate(block, size);

    return block;
}

MemoryBlock* find_block(size_t size)
{
    switch(searchMode)
    {
        case SearchMode::FirstFit:
            return first_fit_search(size);
        case SearchMode::NextFit:
            return next_fit_search(size);
        case SearchMode::BestFit:
            return best_fit_search(size);
    }

    return first_fit_search(size);
}

uint64_t* allocate(size_t size)
{
    size = align(size);
    
    if(MemoryBlock *block = find_block(size))
        return block->data;
    
    MemoryBlock *block = request_block(size);
    
    block->size = size;
    block->isUsed = true;

    if(heapStart == nullptr)
        heapStart = block;

    if(head != nullptr)
        head->next = block;

    head = block;
    searchStart = block;

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
 

    // --------------------------------------
    // Test case 3: Free the object
    //
    free(p2);
    assert(p2b->isUsed == false);


    // --------------------------------------
    // Test case 4: The block is reused
    //
    // A consequent allocation of the same size reuses
    // the previously freed block.
    //
    auto p3 = allocate(8);
    auto p3b = get_header(p3);
    assert(p3b->size == 8);
    assert(p3b == p2b);  // Reused!
    

    // --------------------------------------
    // Test case 5: Next search start position
    //
    // [[8, 1], [8, 1], [8, 1]]
    init(SearchMode::NextFit);

    allocate(8);
    allocate(8);
    allocate(8);
    
    // [[8, 1], [8, 1], [8, 1], [16, 1], [16, 1]]
    auto o1 = allocate(16);
    auto o2 = allocate(16);
    
    // [[8, 1], [8, 1], [8, 1], [16, 0], [16, 0]]
    free(o1);
    free(o2);
    
    // [[8, 1], [8, 1], [8, 1], [16, 1], [16, 0]]
    auto o3 = allocate(16);
    
    // Start position from o3:
    assert(searchStart == get_header(o3));
    
    // [[8, 1], [8, 1], [8, 1], [16, 1], [16, 1]]
    //                           ^ start here
    allocate(16);
    
 
    // --------------------------------------
    // Test case 6: Best-fit search
    //
    init(SearchMode::BestFit);
    
    // [[8, 1], [64, 1], [8, 1], [16, 1]]
    allocate(8);
    auto z1 = allocate(64);
    allocate(8);
    auto z2 = allocate(16);
    
    // Free the last 16
    free(z2);
    
    // Free 64:
    free(z1);
    
    // [[8, 1], [64, 0], [8, 1], [16, 0]]
    
    // Reuse the last 16 block:
    auto z3 = allocate(16);
    assert(get_header(z3) == get_header(z2));
    
    // [[8, 1], [64, 0], [8, 1], [16, 1]]
    
    // Reuse 64, splitting it to 16, and 48
    z3 = allocate(16);
    assert(get_header(z3) == get_header(z1));
    
    // [[8, 1], [16, 1], [48, 0], [8, 1], [16, 1]]

    puts("\nAll assertions passed!\n");
    return 0;
}