#include "alloc.hpp"

MemoryBlock *heapStart = nullptr;
MemoryBlock *head = heapStart;
MemoryBlock *searchStart = heapStart;

SearchMode searchMode = SearchMode::FirstFit;

std::list<MemoryBlock* > freeList;

void reset_heap() 
{
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
    reset_heap();
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

    if(searchMode == SearchMode::FreeList)
        freeList.push_back(block);
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

    return list_allocate(block, size);
}

MemoryBlock* free_list_search(size_t size) 
{
    for (const auto &block : freeList) 
    {
        if (block->size < size) 
            continue;
    
        freeList.remove(block);
        return list_allocate(block, size);
    }
    
    return nullptr;
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
        case SearchMode::FreeList:
            return free_list_search(size);
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