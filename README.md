# Memory allocator
This is a implementation of a dynamic memory allocator written in C++. 
It provides functionality to allocate and free blocks of memory of variable size. The allocator uses the system call ```sbrk()```
to obtain memory from the operating system.

## Overview

The allocator is capable of allocating blocks of memory. It supports different allocation strategies:
- First Fit
- Next Fit
- Best Fit
- Free List

## Usage

To use the allocator, call the `init()` function with needed allocation strategy, `allocate()` function with size as an argument to allocate block and `free()` 
function with a pointer to the memory to be freed as an argument.

```cpp

int main()
{
    init(SearchMode::FirstFit);
    
    int* ptr = (int*)allocate(sizeof(int));
    // ...
    
    free(ptr);
    // ...
    return 0;
}
```

## Search modes

### First-fit
In this mode, the allocator traverse all blocks for the first one that is large enough to satisfy the allocation request.
```cpp
init(SearchMode::FirstFit);
```


### Next-fit
In this mode, the allocator keeps track of the last block that was searched and starts the search from that block 
the next time an allocation request is made.
```cpp
init(SearchMode::NextFit);
```

### Best-fit
In this mode, the allocator traverse all blocks for one that is the closest in size to the allocation request and also split the block of 
memory if it is possible.
```cpp
init(SearchMode::BestFit);
```

### Free List
In this mode, the allocator maintains a list of all free blocks. When an allocation request is made, the allocator 
searches the list for a block that is large enough to satisfy the request.
```cpp
init(SearchMode::FreeList);
```

## How to run
> minimum required version of c++ is c++14


### Run main
```bash
g++ -std=c++17 -c src/main.cpp
g++ -std=c++17 -c src/alloc.cpp
g++ -std=c++17 main.o alloc.o -o main
./main
```

### Run tests
```bash
g++ -std=c++17 -c tests/tests.cpp
g++ -std=c++17 -c src/alloc.cpp
g++ -std=c++17 tests.o alloc.o -o test -lgtest -lgtest_main
./tests
```
