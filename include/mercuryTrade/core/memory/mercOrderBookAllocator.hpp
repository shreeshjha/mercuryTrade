#ifndef MERC_ORDER_BOOK_ALLOCATOR_HPP
#define MERC_ORDER_BOOK_ALLOCATOR_HPP

#include "mercAllocatorManager.hpp"
#include <atomic>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <mutex>

namespace mercuryTrade {
namespace core {
namespace memory {

// Forward declarations for internal linkage
struct OrderNode;
struct PriceLevel;

class OrderBookAllocator {
public:
    // Configuration for order book memory pools
    struct Config {
        std::size_t max_orders;        // Maximum number of orders
        std::size_t max_price_levels;  // Maximum number of price levels
        std::size_t order_data_size;   // Size of additional order data
        bool track_modifications;       // Whether to track order modifications

        // Default configuration
        static Config getDefaultConfig() {
            return Config{
                100000,  // max_orders
                10000,   // max_price_levels
                128,     // order_data_size
                true     // track_modifications
            };
        }
    };

    // Statistics about order book memory usage
    struct Stats {
        std::size_t active_orders;
        std::size_t active_price_levels;
        std::size_t total_memory_used;
        std::size_t available_order_slots;
        std::size_t available_price_slots;
        std::size_t order_modifications;
        std::size_t peak_orders;
        std::size_t peak_memory;
    };

    // Constructor with configuration
    explicit OrderBookAllocator(const Config& config = Config::getDefaultConfig());
    
    // Prevent copying
    OrderBookAllocator(const OrderBookAllocator&) = delete;
    OrderBookAllocator& operator=(const OrderBookAllocator&) = delete;
    
    // Allow moving
    OrderBookAllocator(OrderBookAllocator&&) noexcept = default;
    OrderBookAllocator& operator=(OrderBookAllocator&&) noexcept = default;
    
    // Destructor
    ~OrderBookAllocator() noexcept;

    // Order management
    OrderNode* allocateOrder();
    void deallocateOrder(OrderNode* order);
    
    // Price level management
    PriceLevel* allocatePriceLevel();
    void deallocatePriceLevel(PriceLevel* level);
    
    // Order lookup and tracking
    OrderNode* findOrder(const std::string& order_id);
    void registerOrder(const std::string& order_id, OrderNode* order);
    void unregisterOrder(const std::string& order_id);
    
    // Utility methods
    void reset();  // Clear all allocations
    Stats getStats() const;
    bool hasCapacity() const;

private:
    AllocatorManager m_allocator;
    Config m_config;
    
    // Memory pools
    void* m_order_pool;
    void* m_price_level_pool;

    void cleanupOrderPool();
    void cleanupPriceLevelPool();

    std::size_t m_order_pool_size;
    std::size_t m_price_level_pool_size;
    
    // Statistics tracking
    std::atomic<std::size_t> m_active_orders{0};
    std::atomic<std::size_t> m_active_price_levels{0};
    std::atomic<std::size_t> m_order_modifications{0};
    std::atomic<std::size_t> m_peak_orders{0};
    std::atomic<std::size_t> m_peak_memory{0};
    
    // Fast lookup for order management
    std::unordered_map<std::string, OrderNode*> m_order_map;
    std::mutex m_order_map_mutex; //Add mutex for protecting m_order_map

    // Helper method for memory calculation
    std::size_t calculateTotalMemoryUsed() const;
};

// Order book data structures
struct OrderNode {
    double price;
    double quantity;
    std::string order_id;
    OrderNode* next;
    OrderNode* prev;
    PriceLevel* parent_level;
    char additional_data[];  // Flexible array member for extra data
};

struct PriceLevel {
    double price;
    double total_quantity;
    std::size_t order_count;
    OrderNode* first_order;
    OrderNode* last_order;
    PriceLevel* next;
    PriceLevel* prev;
};

}}} // namespaces

#endif // MERC_ORDER_BOOK_ALLOCATOR_HPP
