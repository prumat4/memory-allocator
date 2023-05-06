#include <iostream>
#include <list>
#include <unistd.h>
#include <cassert>  

#define SIZE_T_MAX 65535

enum class SearchMode
{
    FirstFit,
    NextFit,
    BestFit,
    FreeList
};

struct MemoryBlock
{
    size_t size;
    bool isUsed { false };
    MemoryBlock *next;
    uint64_t data[1];
};

void reset_heap();
void init(SearchMode mode);

size_t align(size_t n);
size_t alloc_size(size_t size);

MemoryBlock* request_block(size_t size);
MemoryBlock* get_header(uint64_t *data);

void free(uint64_t *data);

bool can_split(MemoryBlock *block, size_t size);
MemoryBlock* split(MemoryBlock* block, size_t size);

MemoryBlock* list_allocate(MemoryBlock *block, size_t size);
MemoryBlock* first_fit_search(size_t size);
MemoryBlock* next_fit_search(size_t size);
MemoryBlock* best_fit_search(size_t size);
MemoryBlock* free_list_search(size_t size);

MemoryBlock* find_block(size_t size);
uint64_t* allocate(size_t size);