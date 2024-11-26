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

void tearDown(tradingManager& manager) {
    manager.stop();
    // Force cleanup of any remaining resources
    manager.cleanupResources();
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

void testBasicOrderSubmission(){
    // const char* TEST_NAME = "Basic Order Allocation Test";
    // OrderBookAllocator allocator;

    // try {
    //     // Allocate an order
    //     OrderNode* order = allocator.allocateOrder();
    //     verify(order != nullptr, TEST_NAME, "Order allocation failed");

    //     // Set order properties
    //     order->price = 100.0;
    //     order->quantity = 10.0;
    //     order->order_id = "ORDER1";

    //     // Register and verify order
    //     allocator.registerOrder(order->order_id, order);
    //     verify(allocator.findOrder("ORDER1") == order, TEST_NAME, "Order lookup failed");

    //     // Deallocate and verify cleanup
    //     allocator.deallocateOrder(order);
    //     verify(allocator.getStats().active_orders == 0, TEST_NAME, "Order deallocation failed");
    //     verify(allocator.findOrder("ORDER1") == nullptr, TEST_NAME, "Order still found after deallocation");
    // } catch (...) {
    //     allocator.reset(); // Emergency cleanup
    //     throw;
    // }

        const char* TEST_NAME = "Concurrent Operations Test";
        std::cout << "[DEBUG] Starting " << TEST_NAME << "\n";

        tradingManager manager;
        verify(manager.start(), TEST_NAME, "Failed to start trading system");

        std::atomic<bool> test_failed{false};
        std::atomic<int> completed_threads{0};
        const int NUM_THREADS = 4;
        const int ITERATIONS_PER_THREAD = 10;

        auto thread_func = [&](int thread_id) {
            try {
                for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                    std::string order_id = "ORDER_" + std::to_string(thread_id) + "_" + std::to_string(i);

                    order ord = createTestOrder(order_id, "AAPL", 150.0, 100.0, true);
                    if (!manager.submitOrder(ord)) {
                        std::cerr << "[DEBUG] Thread " << thread_id << " failed to submit order: " << order_id << "\n";
                        test_failed = true;
                        return;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    if (!manager.cancelOrder(order_id)) {
                        std::cerr << "[DEBUG] Thread " << thread_id << " failed to cancel order: " << order_id << "\n";
                        test_failed = true;
                        return;
                    }
                }
                completed_threads.fetch_add(1);
            } catch (const std::exception& e) {
                std::cerr << "[DEBUG] Exception in thread " << thread_id << ": " << e.what() << "\n";
                test_failed = true;
            }
        };

        // Launch threads
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back(thread_func, i);
        }

        // Wait for threads
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Final checks
        if (test_failed || completed_threads.load() != NUM_THREADS) {
            std::cerr << "[DEBUG] " << TEST_NAME << " failed. Completed threads: " << completed_threads.load() << "\n";
            throw std::runtime_error(TEST_NAME + std::string(" failed"));
        }

        verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
        std::cout << "[DEBUG] " << TEST_NAME << " PASSED.\n";
        tearDown(manager);
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
    tearDown(manager);
}

void testSystemStateTransitions(){
    // const char* TEST_NAME = "System State Transitions Test";
    // tradingManager manager;

    // // Test Start
    // verify(manager.start(), TEST_NAME, "Failed to start trading system");
    // verify(manager.getStatus() == tradingManager::Status::RUNNING, TEST_NAME, "Sytem should be running");

    // // Test Pause
    // verify(manager.pause(), TEST_NAME, "Failed to start trading system");
    // verify(manager.getStatus() == tradingManager::Status::PAUSED, TEST_NAME, "System should be paused");

    // // Test resume
    // verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
    // verify(manager.getStatus() == tradingManager::Status::RUNNING, TEST_NAME, "System should be running after resume");

    // // Test stop
    // verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
    // verify(manager.getStatus() == tradingManager::Status::STARTING, TEST_NAME, "System should be in starting state after stop");
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
    tearDown(manager);
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
    tearDown(manager);
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
    tearDown(manager);
}

int main(){
    std::cout << "\nStarting Trading Manager Tests...\n" <<std::endl;
    try{
        testBasicOrderSubmission();
        testMarketDataHandling();
        testSystemStateTransitions();
        testConcurrentOperations();
        testMemoryOptimization();

        std::cout << "\nAll trading manager tests completed successfully!\n" << std::endl;
        return 0;
    }catch (const std::exception& e){
        std::cerr<<"\nTest failed: " << e.what() << std::endl;
        return 1;
    }
}
