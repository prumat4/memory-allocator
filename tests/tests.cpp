#include "../src/alloc.hpp"
#include <gtest/gtest.h>

extern MemoryBlock *heapStart;
extern MemoryBlock *head;
extern MemoryBlock *searchStart;

extern SearchMode searchMode;

extern std::list<MemoryBlock* > freeList;

class AllocatorTests : public testing::Test
{
public:
    uint64_t *obj;
    MemoryBlock *obj_header;
    
    void SetUp() override 
    { 
        obj = allocate(8);
        obj_header = get_header(obj);
    }

    
};

TEST_F(AllocatorTests, AlignEightTest) 
{
    EXPECT_EQ(obj_header->size, 8);    
}

TEST_F(AllocatorTests, FreeTest)
{
    EXPECT_TRUE(obj_header->isUsed);
    free(obj);
    EXPECT_FALSE(obj_header->isUsed);
}

TEST_F(AllocatorTests, ReuseBlockTest)
{
    free(obj);
    uint64_t *obj2 = allocate(8);
    MemoryBlock *obj2_header = get_header(obj2);

    EXPECT_EQ(obj_header, obj2_header);
}

TEST(SearcModesTests, NextFitSearchStartPosition)
{
    init(SearchMode::NextFit);

    allocate(8);
    allocate(8);
    allocate(8);

    uint64_t *obj1 = allocate(16);
    uint64_t *obj2 = allocate(16);

    free(obj1);
    free(obj2);

    uint64_t *obj3 = allocate(16);

    EXPECT_EQ(searchStart, get_header(obj3));
}   

TEST(SearcModesTests, BestFitSearch)
{
    init(SearchMode::BestFit);

    allocate(8);
    uint64_t *z1 = allocate(64);
    
    allocate(8);
    uint64_t *z2 = allocate(16);

    free(z2);
    free(z1);

    uint64_t *z3 = allocate(16);
    EXPECT_EQ(get_header(z3), get_header(z2));

    z3 = allocate(16);
    EXPECT_EQ(get_header(z3), get_header(z1));
}

int main()
{
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}