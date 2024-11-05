#include "mercuryTrade/core/memory/mercAllocatorManager.hpp"
#include <thread>
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_set>

using namespace mercuryTrade::core::memory;

class testFailure : public std::runtime_error{
    public:
        explicit testFailure(const std::string &msg) : std::runtime_error(msg) {}
};

void verify(bool condition, const char* testName, const char* message){
    if (!condition){
        std::cerr<< testName << ": Failed - "<<message<<std::endl;
        throw testFailure(std::string(testName) + "failed: " + message);
    }
    std::cout<< testName <<" : Passed" << std::endl;
}

// Test basic allocation and deallocation
void testBasicAllocation() {
    const char* TEST_NAME = "Basic Allocation Test";
    AllocatorManager manager;

    // test different sizes
    std::vector<std::pair<void*,size_t>> allocations;
    std::vector<size_t> testSizes = {8,16,32,64,128,256,512,1024,2048,4096};
    for (size_t size :  testSizes){
        void* ptr = manager.allocate(size);
        verify(ptr != nullptr, TEST_NAME, "Allocation should succeed");
        allocations.emplace_back(ptr,size);
    }

    // Verify memory usage
    size_t totalMemory = manager.getTotalMemoryUsed();
    verify(totalMemory > 0, TEST_NAME, "Total memory used should be greater than 0");

    // Deallocate everything
    for (const auto &[ptr,size] : allocations){
        manager.deallocate(ptr,size);
    }

    // Verify all memory is freed
    verify(manager.getTotalMemoryUsed() == 0, TEST_NAME, "All memory should be freed after deallocation");
}

// Test pool statistics
void testPoolStatistics(){
    const char* TEST_NAME = "Pool statistics test";
    AllocatorManager manager;

    // Allocate some memory
    void* ptr1 = manager.allocate(32);
    void* ptr2 = manager.allocate(32);
    void* ptr3 = manager.allocate(64);

    auto stats = manager.getPoolStats();
    verify(!stats.empty(), TEST_NAME, "Should have pool statistics");

    // Find 32 byte pool
    auto it32 = std::find_if(stats.begin(),stats.end(),[](const AllocatorManager::PoolStats &s){return s.block_size == 32;});
    verify(it32 != stats.end(), TEST_NAME, "Should find 32 byte pool");
    verify(it32 -> blocks_in_use == 2, TEST_NAME, "Should have 32 bytes in 32-byte pool");

    // cleanup
    manager.deallocate(ptr1,32);
    manager.deallocate(ptr2,32);
    manager.deallocate(ptr3,64);
}