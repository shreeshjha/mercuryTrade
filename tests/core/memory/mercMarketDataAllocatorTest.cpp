#include "../../../include/mercuryTrade/core/memory/mercMarketDataAllocator.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>

using namespace mercuryTrade::core::memory;

void verify(bool condition, const char* testName, const char* message){
    if (!condition){
        std::cerr<< testName << " : Failed - " << message << std::endl;
        throw std::runtime_error(std::string(testName) + " failed : " + message);
    }
    std::cout << testName << " : Passed " << std::endl;
}

// Test Basic Allocation Functionality
void testBasicAllocation() {
    const char* TEST_NAME = "Basic Market Data Allocation Test";
    marketDataAllocator allocator;

    // Allocate different types of buffers
    void* quoteBuffer = allocator.allocateQuoteBuffer();
    void* tradeBuffer = allocator.allocateTradeBuffer();
    void* snapshotBuffer = allocator.allocateSnapshotBuffer();

    verify(quoteBuffer != nullptr , TEST_NAME, "Quote Buffer Allocation Failed");
    verify(tradeBuffer != nullptr, TEST_NAME, "Trade Buffer Allocation Failed");
    verify(snapshotBuffer != nullptr, TEST_NAME, "Snapshot Buffer Allocation Failed");

    // Check statistics
    auto stats = allocator.getStats();
    verify(stats.quotes_allocated == 1, TEST_NAME, "Quote allocation count mismatch");
    verify(stats.trades_allocated == 1, TEST_NAME, "Trade allocation count mismatch");
    verify(stats.snapshots_allocated == 1, TEST_NAME, "Snapshot allocation count mismatch");

    // Deallocate Buffers
    auto config = marketDataAllocator::getDefaultConfig();
    allocator.deallocateBuffer(quoteBuffer, config.quote_size * config.buffer_capacity);
    allocator.deallocateBuffer(tradeBuffer, config.trade_size * config.buffer_capacity);
    allocator.deallocateBuffer(snapshotBuffer, config.snapshot_size * config.buffer_capacity);  

    // Verify deallocation
    stats = allocator.getStats();
    verify(stats.quotes_allocated == 0, TEST_NAME, "Quote Deallocation Failed");
    verify(stats.trades_allocated == 0 , TEST_NAME, "Trade Deallocation Failed");
    verify(stats.snapshots_allocated == 0, TEST_NAME, "Snapshot Deallocation Failed");
}

// Test Concurrent Allocations
void testConcurrentAllocation() {
    const char* TEST_NAME = "Concurrent Market Data Allocation Test";
    
    marketDataAllocator allocator;
    std::atomic<bool> test_failed{false};
    std::mutex buffers_mutex;  // Add mutex for vector access
    std::vector<std::pair<void*, std::size_t>> all_buffers;  // Track all allocations with their sizes
    
    auto thread_func = [&](int thread_id) {
        try {
            std::vector<std::pair<void*, std::size_t>> thread_buffers;
            auto config = marketDataAllocator::getDefaultConfig();
            
            // Each thread allocates multiple buffers
            for (int i = 0; i < 10; ++i) {
                void* buffer = nullptr;
                std::size_t size = 0;
                
                switch (thread_id % 3) {
                    case 0:
                        buffer = allocator.allocateQuoteBuffer();
                        size = config.quote_size * config.buffer_capacity;
                        break;
                    case 1:
                        buffer = allocator.allocateTradeBuffer();
                        size = config.trade_size * config.buffer_capacity;
                        break;
                    case 2:
                        buffer = allocator.allocateSnapshotBuffer();
                        size = config.snapshot_size * config.buffer_capacity;
                        break;
                }
                
                if (!buffer) {
                    test_failed = true;
                } else {
                    thread_buffers.emplace_back(buffer, size);
                }
            }
            
            // Store all allocations
            {
                std::lock_guard<std::mutex> lock(buffers_mutex);
                all_buffers.insert(all_buffers.end(), 
                                 thread_buffers.begin(), 
                                 thread_buffers.end());
            }
            
        } catch (...) {
            test_failed = true;
        }
    };
    
    // Create multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(thread_func, i);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Cleanup all allocations
    for (const auto& [buffer, size] : all_buffers) {
        allocator.deallocateBuffer(buffer, size);
    }
    
    verify(!test_failed, TEST_NAME, "Concurrent allocation test failed");
    
    // Verify final state
    auto stats = allocator.getStats();
    verify(stats.total_memory_used == 0, TEST_NAME, "Memory leak detected");
    verify(stats.quotes_allocated == 0, TEST_NAME, "Quote allocations not properly cleaned up");
    verify(stats.trades_allocated == 0, TEST_NAME, "Trade allocations not properly cleaned up");
    verify(stats.snapshots_allocated == 0, TEST_NAME, "Snapshot allocations not properly cleaned up");
}

// Test Invalid Configurations
void testInvalidConfiguration() {
  const char* TEST_NAME = "Invalid Configuration Test";
    bool exception_thrown = false;

    try {
        marketDataAllocator::bufferConfig invalid_config{};  // Zero-initialized
        [[maybe_unused]] marketDataAllocator allocator(invalid_config);
    } catch (...) {  // Catch any exception
        exception_thrown = true;
    }
    verify(exception_thrown, TEST_NAME, "Should throw on invalid configuration");
}
int main(){
    std::cout << "\n Starting market data allocator test...\n" << std::endl;
    try{
        testBasicAllocation();
        testConcurrentAllocation();
        testInvalidConfiguration();

        std::cout << "\n All market data allocator tests completed successfully\n" << std::endl;
        return 0;
    } catch (const std::exception &e){
        std::cerr << "\n Test Failed \n" << std::endl;
        return 1;
    }
}
