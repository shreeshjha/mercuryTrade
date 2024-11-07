#include "../../../include/mercuryTrade/core/memory/mercAllocatorManager.hpp"
#include <thread>
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <cstring>


using namespace mercuryTrade::core::memory;

class TestFailure : public std::runtime_error {
public:
    explicit TestFailure(const std::string& msg) : std::runtime_error(msg) {}
};

void verify(bool condition, const char* testName, const char* message) {
    if (!condition) {
        std::cerr << testName << ": FAILED - " << message << std::endl;
        throw TestFailure(std::string(testName) + " failed: " + message);
    }
    std::cout << testName << ": PASSED" << std::endl;
}

// Test basic allocation and deallocation
void testBasicAllocation() {
    const char* TEST_NAME = "Basic Allocation Test";
    
    AllocatorManager manager;
    
    // Test different sizes
    std::vector<std::pair<void*, size_t>> allocations;
    std::vector<size_t> testSizes = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    
    for (size_t size : testSizes) {
        void* ptr = manager.allocate(size);
        verify(ptr != nullptr, TEST_NAME, "Allocation should succeed");
        allocations.emplace_back(ptr, size);
    }
    
    // Verify memory usage
    size_t totalMemory = manager.getTotalMemoryUsed();
    verify(totalMemory > 0, TEST_NAME, "Total memory used should be greater than 0");
    
    // Deallocate everything
    for (const auto& [ptr, size] : allocations) {
        manager.deallocate(ptr, size);
    }
    
    // Verify all memory is freed
    verify(manager.getTotalMemoryUsed() == 0, TEST_NAME, 
           "All memory should be freed after deallocation");
}

// Test pool statistics
void testPoolStatistics() {
    const char* TEST_NAME = "Pool Statistics Test";
    
    AllocatorManager manager;
    std::vector<std::pair<void*, size_t>> allocations;
    
    // Allocate some memory
    void* ptr1 = manager.allocate(32);
    void* ptr2 = manager.allocate(32);
    void* ptr3 = manager.allocate(64);
    
    auto stats = manager.getPoolStats();
    verify(!stats.empty(), TEST_NAME, "Should have pool statistics");
    
    // Find 32-byte pool
    auto it32 = std::find_if(stats.begin(), stats.end(),
        [](const AllocatorManager::PoolStats& s) { return s.block_size == 32; });
    
    verify(it32 != stats.end(), TEST_NAME, "Should find 32-byte pool");
    verify(it32->blocks_in_use == 2, TEST_NAME, "Should have 2 blocks in use in 32-byte pool");
    
    // Cleanup
    manager.deallocate(ptr1, 32);
    manager.deallocate(ptr2, 32);
    manager.deallocate(ptr3, 64);
}

// Test multithreaded allocations
void testMultithreadedAllocation() {
    const char* TEST_NAME = "Multithreaded Allocation Test";
    
    AllocatorManager manager;
    const size_t numThreads = 4;
    const size_t allocsPerThread = 100;
    std::atomic<bool> testPassed{true};
    
    auto threadFunc = [&]() {
        try {
            std::vector<std::pair<void*, size_t>> threadAllocations;
            std::vector<size_t> sizes = {16, 32, 64, 128, 256};
            
            for (size_t i = 0; i < allocsPerThread; ++i) {
                size_t size = sizes[i % sizes.size()];
                void* ptr = manager.allocate(size);
                if (!ptr) {
                    testPassed = false;
                    return;
                }
                threadAllocations.emplace_back(ptr, size);
                
                // Randomly deallocate some allocations
                if (!threadAllocations.empty() && (rand() % 2 == 0)) {
                    size_t index = rand() % threadAllocations.size();
                    auto [ptr, size] = threadAllocations[index];
                    manager.deallocate(ptr, size);
                    threadAllocations[index] = threadAllocations.back();
                    threadAllocations.pop_back();
                }
            }
            
            // Cleanup remaining allocations
            for (const auto& [ptr, size] : threadAllocations) {
                manager.deallocate(ptr, size);
            }
        } catch (...) {
            testPassed = false;
        }
    };
    
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunc);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    verify(testPassed, TEST_NAME, "All thread operations should complete successfully");
    verify(manager.getTotalMemoryUsed() == 0, TEST_NAME, 
           "All memory should be freed after thread completion");
}

// Test large allocations (above MAX_BLOCK_SIZE)
void testLargeAllocations() {
    const char* TEST_NAME = "Large Allocation Test";
    
    AllocatorManager manager;
    const size_t largeSize = 1024 * 1024; // 1MB
    
    // Test large allocation
    void* ptr = manager.allocate(largeSize);
    verify(ptr != nullptr, TEST_NAME, "Large allocation should succeed");
    
    // Try writing to the memory to ensure it's valid
    std::memset(ptr, 0xFF, largeSize);
    
    // Cleanup
    manager.deallocate(ptr, largeSize);
}

int main() {
    std::cout << "\nStarting Mercury Allocator Manager Tests...\n" << std::endl;
    
    try {
        testBasicAllocation();
        testPoolStatistics();
        testMultithreadedAllocation();
        testLargeAllocations();
        
        std::cout << "\nAll tests completed successfully!\n" << std::endl;
        return 0;
    }
    catch (const TestFailure& e) {
        std::cerr << "\nTest failed: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "\nUnexpected error: " << e.what() << std::endl;
        return 1;
    }
}
