#include "../../../include/mercuryTrade/core/memory/mercAllocator.hpp"
#include <thread>
#include <vector>
#include <cassert>
#include <iostream>

using namespace mercuryTrade::core::memory

//Test helper to print results
void printTestResult(const char* testName, bool passed) {
  std::cout << testName << ": " << (passed ? "PASSED" : "FAILED") << std::endl;
}

//Basic allocation test 

void testBasicAllocation() {
  FixedAllocator allocator(4) //Small pool for testing 
  
  //Test initial state
  assert(allocator.block_in_use() == 0);
  assert(allocator.available_blocks() == 4);

  //Test allocation
  void* ptr1 = allocator.allocate();
  assert(ptr1 != nullptr);
  assert(allocator.block_in_use() == 1);
  assert(allocator.available_blocks() == 3);

  //Test deallocation
  allocator.deallocate(ptr1);
  assert(allocator.block_in_use() == 0);
  assert(allocator.available_blocks() == 4);

  printTestResult("Basic Allocation", true);
}

//Test full allocation and out of memory condition
void testFullAllocation() {
  const size_t poolSize = 2;
  FixedAllocator allocator(poolSize);
  std::vector<void*> ptrs;

  //allocate all blocks
  for(size_t i = 0; i < poolSize; i++) {
    void *ptr = allocator.allocate();
    assert(ptr != nullptr);
    ptrs.push_back(ptr);
  }
  
  //Verify if we're out of memory
  assert(allocator.allocate() == nullptr);
  assert(allocator.block_in_use() == poolSize);
  assert(allocator.available_blocks() == 0);

  //Free all blocks
  for(void *ptr : ptrs) {
    allocator.deallocate(ptr);
  }

  //Verify all memory is available again
  assert(allocator.block_in_use() == 0);
  assert(allocator.available_blocks() == poolSize);

  printTestResult("Full Allocation", true);
}

// Test multi-threaded allocation
void testMultithreadedAllocation() {
    const size_t poolSize = 1000;
    const size_t numThreads = 4;
    const size_t opsPerThread = 250;
    
    FixedAllocator allocator(poolSize);
    std::vector<std::thread> threads;
    
    auto threadFunc = [&]() {
        std::vector<void*> threadPtrs;
        
        // Perform multiple allocations and deallocations
        for (size_t i = 0; i < opsPerThread; ++i) {
            void* ptr = allocator.allocate();
            if (ptr) {
                threadPtrs.push_back(ptr);
            }
            
            // Randomly deallocate some memory
            if (!threadPtrs.empty() && (rand() % 2 == 0)) {
                size_t index = rand() % threadPtrs.size();
                allocator.deallocate(threadPtrs[index]);
                threadPtrs[index] = threadPtrs.back();
                threadPtrs.pop_back();
            }
        }
        
        // Cleanup remaining allocations
        for (void* ptr : threadPtrs) {
            allocator.deallocate(ptr);
        }
    };
    
    // Start threads
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunc);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify final state
    assert(allocator.blocks_in_use() == 0);
    assert(allocator.available_blocks() == poolSize);
    
    printTestResult("Multithreaded Allocation", true);
}

// Test move semantics
void testMoveSemantics() {
    FixedAllocator allocator1(4);
    
    // Allocate some memory
    void* ptr1 = allocator1.allocate();
    void* ptr2 = allocator1.allocate();
    assert(allocator1.blocks_in_use() == 2);
    
    // Move construct
    FixedAllocator allocator2(std::move(allocator1));
    assert(allocator2.blocks_in_use() == 2);
    
    // Original allocator should be empty
    assert(allocator1.blocks_in_use() == 0);
    
    // New allocator should work properly
    allocator2.deallocate(ptr1);
    allocator2.deallocate(ptr2);
    assert(allocator2.blocks_in_use() == 0);
    
    printTestResult("Move Semantics", true);
}

int main() {
    std::cout << "Starting Mercury Allocator Tests...\n" << std::endl;
    
    try {
        testBasicAllocation();
        testFullAllocation();
        testMultithreadedAllocation();
        testMoveSemantics();
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
