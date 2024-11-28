#include "../../../include/mercuryTrade/core/memory/mercTradingManager.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include <chrono>

using namespace mercuryTrade::core::memory;

void verify(bool condition, const char* testName, const char* message){
    if (!condition){
        std::cerr << testName <<" : Failed - " << message << std::endl;
        throw std::runtime_error(std::string(testName) + "failed: " + message);
    }
    std::cout<< testName << ": PASSED" << std::endl;
}

// Helper function to create a test order
order createTestOrder(const std::string& id, const std::string& symbol, double price, double quantity, bool is_buy){
    order ord;
    ord.order_id = id;
    ord.symbol = symbol;
    ord.price = price;
    ord.quantity = quantity;
    ord.is_buy = is_buy;
    ord.timestamp = std::chrono::system_clock::now();
    return ord;
}

// Helper function to create test market data
marketData createTestMarketData(const std::string& symbol, double bid, double ask, double last, double volume){
    marketData data;
    data.symbol = symbol;
    data.bid = bid;
    data.ask = ask;
    data.last = last;
    data.volume = volume;
    data.timestamp = std::chrono::system_clock::now();
    return data;
}

void cleanupTest(tradingManager& manager) {
    try {
        std::cout << "Starting test cleanup..." << std::endl;
        
        // Stop the manager if it's running
        if (manager.getStatus() == tradingManager::Status::RUNNING) {
            if (!manager.stop()) {
                throw std::runtime_error("Failed to stop trading manager");
            }
        }
        
        // Verify cleanup
        auto stats = manager.getStats();
        if (stats.active_orders != 0 || stats.pending_transactions != 0) {
            throw std::runtime_error("Resources not properly cleaned up");
        }
        
        std::cout << "Test cleanup completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test cleanup failed: " << e.what() << std::endl;
        throw;
    }
}
void testBasicOrderSubmission() {
    const char* TEST_NAME = "Basic Order Submission Test";
    try {
        tradingManager manager;
        verify(manager.start(), TEST_NAME, "Failed to start trading system");

        // Create and submit test order
        order ord = createTestOrder("ORDER1", "AAPL", 100.0, 10.0, true);
        verify(manager.submitOrder(ord), TEST_NAME, "Order submission failed");
        
        // Verify the trading system state
        auto stats = manager.getStats();
        verify(stats.active_orders == 1, TEST_NAME, "Active orders count mismatch");
        
        verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
        cleanupTest(manager);
    } catch (const std::exception& e) {
        std::cerr << TEST_NAME << " failed with exception: " << e.what() << std::endl;
        throw;
    }
}
void testMarketDataHandling(){
    const char* TEST_NAME = "Market Data Handling Test";
    tradingManager manager;
    verify(manager.start(), TEST_NAME, "Failed to start trading system");
    
    // Submit market data
    marketData data = createTestMarketData("AAPL", 150.0, 150.1, 150.05, 1000.0);
    manager.handleMarketData(data);

    // Check system health after market data processing
    verify(manager.ishealthy(), TEST_NAME, "System should be healthy after market data");

    // Submit an order after market data update
    order ord = createTestOrder("ORDER2", "AAPL", 150.0, 100.0, true);
    verify(manager.submitOrder(ord), TEST_NAME, "Order submission after market data failed");

    verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
    cleanupTest(manager);
}

void testSystemStateTransitions(){
    const char* TEST_NAME = "System State Transitions Test";
    tradingManager manager;

    // Test Start
    verify(manager.start(), TEST_NAME, "Failed to start trading system");
    verify(manager.getStatus() == tradingManager::Status::RUNNING, TEST_NAME, "System should be running");

    // Test Pause
    verify(manager.pause(), TEST_NAME, "Failed to pause trading system");
    verify(manager.getStatus() == tradingManager::Status::PAUSED, TEST_NAME, "System should be paused");

    // Test Resume
    verify(manager.resume(), TEST_NAME, "Failed to resume trading system");
    verify(manager.getStatus() == tradingManager::Status::RUNNING, TEST_NAME, "System should be running after resume");

    // Test Stop
    verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
    verify(manager.getStatus() == tradingManager::Status::STARTING, TEST_NAME, "System should be in starting state after stop");
    cleanupTest(manager);
}

void testConcurrentOperations() {
    const char* TEST_NAME = "Concurrent Operations Test";

    tradingManager manager;
    verify(manager.start(), TEST_NAME, "Failed to start trading system");

    std::atomic<bool> test_failed{false};
    std::atomic<int> completed_threads{0};
    std::mutex test_mutex;
    const int ITERATIONS_PER_THREAD = 10;

    auto thread_func = [&](int thread_id) {
        std::cout << "Thread " << thread_id << " started" << std::endl;
        int successful_orders = 0;
        
        try {
            for (int i = 0; i < ITERATIONS_PER_THREAD && !test_failed; i++) {
                std::string order_id = "ORDER_" + std::to_string(thread_id) + "_" + std::to_string(i);
                
                {
                    std::lock_guard<std::mutex> lock(test_mutex);
                    std::cout << "Thread " << thread_id << " iteration " << i << " started" << std::endl;
                }

                order ord = createTestOrder(order_id, "AAPL", 150.0, 100.0, true);
                if (manager.submitOrder(ord)) {
                    successful_orders++;
                } else {
                    std::lock_guard<std::mutex> lock(test_mutex);
                    std::cout << "Thread " << thread_id << " failed to submit order " << order_id << std::endl;
                    continue;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            {
                std::lock_guard<std::mutex> lock(test_mutex);
                std::cout << "Thread " << thread_id << " completed with " 
                         << successful_orders << " successful orders" << std::endl;
            }
            
            completed_threads.fetch_add(1);
            
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(test_mutex);
            std::cout << "Thread " << thread_id << " failed with exception: " << e.what() << std::endl;
            test_failed = true;
        }
    };

    std::vector<std::thread> threads;
    const int NUM_THREADS = 4;
    threads.reserve(NUM_THREADS);

    std::cout << "Starting " << NUM_THREADS << " threads..." << std::endl;

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(thread_func, i);
    }

    // Wait for threads with timeout
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout_duration = std::chrono::seconds(5);
    bool all_threads_completed = false;

    while (std::chrono::steady_clock::now() - start_time < timeout_duration) {
        int current_completed = completed_threads.load();
        std::cout << "Completed threads: " << current_completed << "/" << NUM_THREADS << std::endl;
        
        if (current_completed == NUM_THREADS) {
            all_threads_completed = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!all_threads_completed) {
        std::cout << "Test timed out. Only " << completed_threads.load() 
                 << " of " << NUM_THREADS << " threads completed." << std::endl;
        test_failed = true;
    }

    // Simple thread joining with timeout
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();  // Just do a regular join since we've already handled the timeout
        }
    }

    // Final verification
    verify(!test_failed, TEST_NAME, "Concurrent Operations test failed");
    verify(all_threads_completed, TEST_NAME, "Not all threads completed within timeout");
    verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
    // Print final stats
    auto stats = manager.getStats();
    std::cout << "Final Statistics:" << std::endl
              << "Active Orders: " << stats.active_orders << std::endl
              << "Total Trades: " << stats.total_trades << std::endl
              << "Pending Transactions: " << stats.pending_transactions << std::endl
              << "Average Latency: " << stats.avg_latency << std::endl;
    std::cout << "Test completed and resources cleaned up" << std::endl;
}
void testMemoryOptimization(){
    const char* TEST_NAME = "Memory Optimization Test";

    tradingManager manager;
    verify(manager.start(), TEST_NAME, "Failed to start trading system");

    // Submit multiple orders for consume memory
    for (int i=0; i < 100; i++){
        order ord = createTestOrder("ORDER_" + std::to_string(i), "AAPL", 150.0, 100.0, true);
        manager.submitOrder(ord);
    }

    // Get memory usage before optimization
    auto stats_before = manager.getStats();

    // Perform optimization
    manager.optimizeMemory();

    // Get memory usage after optimization
    auto stats_after = manager.getStats();

    // Verify memory was optimized
    verify(stats_after.memory_used <= stats_before.memory_used, TEST_NAME, "Memory usage should mot increase after optimization");
    verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
    cleanupTest(manager);
}

int main() {
    std::cout << "\nStarting Trading Manager Tests...\n" << std::endl;
    
    try {
        // Run each test in a separate try-catch block
        try {
            testBasicOrderSubmission();
            std::cout << "Basic order submission test completed\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Basic order submission test failed: " << e.what() << std::endl;
            throw;
        }

        try {
            testMarketDataHandling();
            std::cout << "Market data handling test completed\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Market data handling test failed: " << e.what() << std::endl;
            throw;
        }

        try {
            testSystemStateTransitions();
            std::cout << "System state transitions test completed\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "System state transitions test failed: " << e.what() << std::endl;
            throw;
        }

        try {
            testConcurrentOperations();
            std::cout << "Concurrent operations test completed\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Concurrent operations test failed: " << e.what() << std::endl;
            throw;
        }

        try {
            testMemoryOptimization();
            std::cout << "Memory optimization test completed\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Memory optimization test failed: " << e.what() << std::endl;
            throw;
        }

        std::cout << "\nAll trading manager tests completed successfully!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest suite failed: " << e.what() << std::endl;
        return 1;
    }
}
