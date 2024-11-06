#include "mercuryTrade/core/memory/mercAllocatorManager.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace mercuryTrade::core::memory;

void verify(bool condition, const char* testName, const char* message) {
    if (!condition) {
        std::cerr << testName << ": FAILED - " << message << std::endl;
        throw std::runtime_error(std::string(testName) + " failed: " + message);
    }
    std::cout << testName << ": PASSED" << std::endl;
}

void testMemoryTracking() {
    const char* TEST_NAME = "Memory Tracking Test";
    
    AllocatorManager manager;
    std::vector<void*> allocations;
    
    // Make some allocations
    for (int i = 0; i < 5; ++i) {
        void* ptr = ALLOC_TRACKED(manager, 32);
        verify(ptr != nullptr, TEST_NAME, "Allocation should succeed");
        allocations.push_back(ptr);
    }
    
    // Check memory stats
    auto stats = manager.getMemoryStats();
    verify(stats.totalAllocations == 5, TEST_NAME, "Should have 5 total allocations");
    verify(stats.activeAllocations == 5, TEST_NAME, "Should have 5 active allocations");
    
    // Free some memory
    manager.deallocate(allocations[0], 32);
    manager.deallocate(allocations[1], 32);
    
    stats = manager.getMemoryStats();
    verify(stats.activeAllocations == 3, TEST_NAME, "Should have 3 active allocations");
    
    // Clean up remaining allocations
    for (size_t i = 2; i < allocations.size(); ++i) {
        manager.deallocate(allocations[i], 32);
    }
    
    stats = manager.getMemoryStats();
    verify(stats.activeAllocations == 0, TEST_NAME, "Should have no active allocations");
}

void testMemoryLeakDetection() {
    const char* TEST_NAME = "Memory Leak Detection Test";
    
    AllocatorManager manager;
    
    // Create some deliberate leaks
    void* ptr1 = ALLOC_TRACKED(manager, 64);
    void* ptr2 = ALLOC_TRACKED(manager, 128);
    
    // Only deallocate one
    manager.deallocate(ptr1, 64);
    
    auto stats = manager.getMemoryStats();
    verify(stats.activeAllocations == 1, TEST_NAME, "Should have one active allocation");
    
    // Clean up for the test
    manager.deallocate(ptr2, 128);
}

int main() {
    std::cout << "\nStarting Memory Tracking Tests...\n" << std::endl;
    
    try {
        testMemoryTracking();
        testMemoryLeakDetection();
        
        std::cout << "\nAll memory tracking tests completed successfully!\n" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\nTest failed: " << e.what() << std::endl;
        return 1;
    }
}
