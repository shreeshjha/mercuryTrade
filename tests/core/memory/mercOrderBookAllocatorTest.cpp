#include "../../../include/mercuryTrade/core/memory/mercOrderBookAllocator.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <sstream>

using namespace mercuryTrade::core::memory;

void verify(bool condition, const char* testName, const char* message) {
    if (!condition) {
        std::cerr << testName << ": FAILED - " << message << std::endl;
        throw std::runtime_error(std::string(testName) + " failed: " + message);
    }
    std::cout << testName << ": PASSED" << std::endl;
}

// Test basic order allocation and deallocation
void testBasicOrderAllocation() {
    const char* TEST_NAME = "Basic Order Allocation Test";
    
    OrderBookAllocator allocator;
    
    // Allocate an order
    OrderNode* order = allocator.allocateOrder();
    verify(order != nullptr, TEST_NAME, "Order allocation failed");
    
    // Set order properties
    order->price = 100.0;
    order->quantity = 10.0;
    order->order_id = "ORDER1";
    
    // Register order
    allocator.registerOrder(order->order_id, order);
    
    // Verify order lookup
    OrderNode* found = allocator.findOrder("ORDER1");
    verify(found == order, TEST_NAME, "Order lookup failed");
    
    // Check statistics
    auto stats = allocator.getStats();
    verify(stats.active_orders == 1, TEST_NAME, "Active orders count mismatch");
    
    // Deallocate order
    allocator.deallocateOrder(order);
    
    // Verify deallocation
    stats = allocator.getStats();
    verify(stats.active_orders == 0, TEST_NAME, "Order deallocation failed");
    verify(allocator.findOrder("ORDER1") == nullptr, TEST_NAME, "Order still found after deallocation");
}

// Test price level management
void testPriceLevelManagement() {
    const char* TEST_NAME = "Price Level Management Test";
    std::cout << "\nStarting " << TEST_NAME << std::endl;
    
    OrderBookAllocator allocator;
    std::vector<OrderNode*> orders;
    PriceLevel* level = nullptr;

    try {
        // Allocate a price level
        std::cout << "Allocating price level..." << std::endl;
        level = allocator.allocatePriceLevel();
        verify(level != nullptr, TEST_NAME, "Price level allocation failed");
        
        // Initialize price level with safe defaults
        level->price = 100.0;
        level->total_quantity = 0.0;
        level->order_count = 0;
        level->first_order = nullptr;
        level->last_order = nullptr;
        level->next = nullptr;
        level->prev = nullptr;
        
        std::cout << "Adding orders to price level..." << std::endl;
        
        const int NUM_ORDERS = 5;
        orders.reserve(NUM_ORDERS);  // Pre-allocate vector space

        // Add orders one by one with careful verification
        for (int i = 0; i < NUM_ORDERS; ++i) {
            std::cout << "Allocating order " << i << std::endl;
            
            // Allocate new order
            OrderNode* order = allocator.allocateOrder();
            verify(order != nullptr, TEST_NAME, "Order allocation failed");
            
            std::cout << "Initializing order " << i << std::endl;
            
            // Generate order ID first
            std::stringstream ss;
            ss << "ORDER" << i;
            order->order_id = ss.str();
            
            // Initialize order
            order->price = 100.0;
            order->quantity = 10.0;
            order->next = nullptr;
            order->prev = nullptr;
            order->parent_level = level;
            
            // Store order for cleanup
            orders.push_back(order);
            
            std::cout << "Linking order " << i << " to price level" << std::endl;
            
            // Link to price level
            if (level->order_count == 0) {
                level->first_order = order;
                level->last_order = order;
            } else {
                if (level->last_order) {
                    order->prev = level->last_order;
                    level->last_order->next = order;
                    level->last_order = order;
                }
            }
            
            // Update price level stats
            level->order_count++;
            level->total_quantity += order->quantity;
            
            // Register order
            std::cout << "Registering order " << i << std::endl;
            allocator.registerOrder(order->order_id, order);
            
            // Verify after each order
            verify(level->order_count == i + 1, TEST_NAME, 
                  "Order count mismatch after adding order");
        }
        
        // Clean up
        std::cout << "Starting cleanup..." << std::endl;
        
        // First unregister all orders
        for (auto* order : orders) {
            if (order && !order->order_id.empty()) {
                allocator.unregisterOrder(order->order_id);
            }
        }
        
        // Then deallocate price level (which should handle the orders)
        if (level) {
            allocator.deallocatePriceLevel(level);
        }
        
        // Verify final state
        auto stats = allocator.getStats();
        verify(stats.active_orders == 0, TEST_NAME, "Orders not properly deallocated");
        verify(stats.active_price_levels == 0, TEST_NAME, "Price level not properly deallocated");
        
        std::cout << TEST_NAME << " completed successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << TEST_NAME << " failed with exception: " << e.what() << std::endl;
        
        // Emergency cleanup
        for (auto* order : orders) {
            if (order) {
                try {
                    allocator.unregisterOrder(order->order_id);
                } catch (...) {}
            }
        }
        
        if (level) {
            try {
                allocator.deallocatePriceLevel(level);
            } catch (...) {}
        }
        
        throw;
    }
}
// Test Concurrent Operations
void testConcurrentOperations() {
    const char* TEST_NAME = "Concurrent Operations Test";
    
    OrderBookAllocator allocator(OrderBookAllocator::Config{
        1000,   // max_orders
        100,    // max_price_levels
        128,    // order_data_size
        true    // track_modifications
    });
    
    std::atomic<bool> test_failed{false};
    std::atomic<int> total_orders{0};
    std::atomic<bool> should_stop{false};  // Add stop signal
    
    auto thread_func = [&](int thread_id) {
        try {
            std::vector<OrderNode*> thread_orders;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> price_dist(1.0, 1000.0);
            std::uniform_real_distribution<> qty_dist(1.0, 100.0);
            
            int operation_count = 0;
            const int MAX_OPERATIONS = 50;  // Limit operations per thread
            
            // Perform random operations with a limit
            while (!should_stop && operation_count < MAX_OPERATIONS) {
                // Randomly allocate or deallocate
                if (thread_orders.size() < 5 && gen() % 2 == 0) {  // Reduced max orders per thread
                    // Allocate new order
                    OrderNode* order = allocator.allocateOrder();
                    if (order) {
                        order->price = price_dist(gen);
                        order->quantity = qty_dist(gen);
                        
                        std::stringstream ss;
                        ss << "ORDER_" << thread_id << "_" << operation_count;
                        order->order_id = ss.str();
                        
                        allocator.registerOrder(order->order_id, order);
                        thread_orders.push_back(order);
                        total_orders++;
                    }
                } else if (!thread_orders.empty()) {
                    // Deallocate random order
                    size_t index = gen() % thread_orders.size();
                    OrderNode* order = thread_orders[index];
                    allocator.deallocateOrder(order);
                    thread_orders[index] = thread_orders.back();
                    thread_orders.pop_back();
                    total_orders--;
                }
                
                operation_count++;
                std::this_thread::yield();  // Allow other threads to run
            }
            
            // Cleanup remaining orders
            for (OrderNode* order : thread_orders) {
                allocator.deallocateOrder(order);
                total_orders--;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Thread " << thread_id << " failed: " << e.what() << std::endl;
            test_failed = true;
            should_stop = true;
        } catch (...) {
            std::cerr << "Thread " << thread_id << " failed with unknown error" << std::endl;
            test_failed = true;
            should_stop = true;
        }
    };
    
    // Create multiple threads
    std::vector<std::thread> threads;
    const int NUM_THREADS = 4;
    threads.reserve(NUM_THREADS);
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(thread_func, i);
    }
    
    // Add timeout
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(5);  // 5 second timeout
    
    // Wait for threads with timeout
    bool timeout_occurred = false;
    for (auto& thread : threads) {
        if (thread.joinable()) {
            auto now = std::chrono::steady_clock::now();
            if (now - start_time > timeout) {
                should_stop = true;
                timeout_occurred = true;
            }
            thread.join();
        }
    }
    
    if (timeout_occurred) {
        throw std::runtime_error("Test timed out after 5 seconds");
    }
    
    // Verify final state
    verify(!test_failed, TEST_NAME, "Concurrent operations test failed");
    verify(total_orders == 0, TEST_NAME, "Order count mismatch after concurrent operations");
    
    auto stats = allocator.getStats();
    verify(stats.active_orders == 0, TEST_NAME, "Memory leak detected");
    verify(stats.active_price_levels == 0, TEST_NAME, "Price level leak detected");
}

// Test capacity limits
void testCapacityLimits() {
    const char* TEST_NAME = "Capacity Limits Test";
    
    // Create allocator with small limits
    OrderBookAllocator allocator(OrderBookAllocator::Config{
        5,      // max_orders
        2,      // max_price_levels
        128,    // order_data_size
        true    // track_modifications
    });
    
    // Test order capacity
    std::vector<OrderNode*> orders;
    for (int i = 0; i < 7; ++i) {  // Try to allocate more than max
        OrderNode* order = allocator.allocateOrder();
        if (order) {
            orders.push_back(order);
        }
    }
    
    verify(orders.size() == 5, TEST_NAME, "Order capacity limit not enforced");
    
    // Test price level capacity
    std::vector<PriceLevel*> levels;
    for (int i = 0; i < 4; ++i) {  // Try to allocate more than max
        PriceLevel* level = allocator.allocatePriceLevel();
        if (level) {
            levels.push_back(level);
        }
    }
    
    verify(levels.size() == 2, TEST_NAME, "Price level capacity limit not enforced");
    
    // Cleanup
    for (auto order : orders) {
        allocator.deallocateOrder(order);
    }
    for (auto level : levels) {
        allocator.deallocatePriceLevel(level);
    }
}

int main() {
    std::cout << "\nStarting Order Book Allocator Tests...\n" << std::endl;
    
    try {
        testBasicOrderAllocation();
        testPriceLevelManagement();
        testConcurrentOperations();
        testCapacityLimits();
        
        std::cout << "\nAll order book allocator tests completed successfully!\n" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\nTest failed: " << e.what() << std::endl;
        return 1;
    }
}
