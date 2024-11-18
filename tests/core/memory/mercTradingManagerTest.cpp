#include "../../../include/mercuryTrade/core/memory/mercTradingManager.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

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

void testBasicOrderSubmission(){
    const char* TEST_NAME = "Basic Order Submission Test";

    tradingManager manager;
    // Start the trading system
    verify(manager.start(), TEST_NAME, "Failed to start trading system");
    verify(manager.getStatus() == tradingManager::Status::RUNNING, TEST_NAME, "Trading System should be running");

    // Submit a valid order
    order ord = createTestOrder("ORDER1" , "AAPL" , 150.0 , 100.0 , true);
    bool result = manager.submitOrder(ord);
    verify(result, TEST_NAME, "Order submission failed");

    // Check stats
    auto stats = manager.getStats();
    verify(stats.active_orders > 0 , TEST_NAME, "Should have active orders");

    // Cancel the order
    result = manager.cancelOrder("ORDER1");
    verify(result, TEST_NAME, "Order cancellation failed");

    // Stop the trading system
    verify(manager.stop(), TEST_NAME , "Failed to stop trading system");
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
}

void testConcurrentOperations(){
    const char* TEST_NAME = "Concurrent Operations Test";

    tradingManager manager;
    verify(manager.start(), TEST_NAME, "Failed to start trading system");

    std::atomic<bool> test_failed{false};
    std::mutex test_mutex;

    auto thread_func = [&](int thread_id){
        try{
            for (int i = 0; i < 100 && !test_failed; i++){
                // Generate unique order ID
                std::string order_id = "ORDER_" + std::to_string(thread_id) + "_" + std::to_string(i);

                // Submit order
                order ord = createTestOrder(order_id, "AAPL", 150.0, 100.0, true);
                if (!manager.submitOrder(ord)){
                    test_failed = true;
                    break;
                }

                // Submit market data
                marketData data = createTestMarketData("AAPL", 150.0, 150.1, 150.05, 1000.0);
                manager.handleMarketData(data);

                // Cancel some orders
                if (i % 2 == 0){
                    manager.cancelOrder(order_id);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }catch (const std::exception& e){
            std::lock_guard<std::mutex> lock(test_mutex);
            std::cerr << "Thread" << thread_id << " failed: " << e.what() << std::endl;
            test_failed = true;
        }
    };

    // Create and run threads
    std::vector<std::thread> threads;
    const int NUM_THREADS = 4;
    threads.reserve(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++){
        threads.emplace_back(thread_func , i);
    }

    // Wait for threads
    for (auto& thread : threads){
        if (thread.joinable()){
            thread.join();
        }
    }

    verify(!test_failed, TEST_NAME, "Concurrent Operations test failed");
    verify(manager.stop(), TEST_NAME, "Failed to stop trading system");
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