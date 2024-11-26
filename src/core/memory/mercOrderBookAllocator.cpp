#include "../../../include/mercuryTrade/core/memory/mercOrderBookAllocator.hpp"
#include <stdexcept>

namespace mercuryTrade {
namespace core {
namespace memory {

OrderBookAllocator::OrderBookAllocator(const Config& config)
    : m_config(config)
    , m_order_pool_size((sizeof(OrderNode) + config.order_data_size) * config.max_orders)  
    , m_price_level_pool_size(sizeof(PriceLevel) * config.max_price_levels)
{
    if (config.max_orders == 0 || config.max_price_levels == 0) {
        throw std::invalid_argument("Invalid order book configuration");
    }

    // Allocate order pool
    // std::size_t order_size = sizeof(OrderNode) + config.order_data_size;
    // m_order_pool = m_allocator.allocate(order_size * config.max_orders);

    m_order_pool = m_allocator.allocate(m_order_pool_size);

    // Allocate price level pool
    // m_price_level_pool = m_allocator.allocate(
    //     sizeof(PriceLevel) * config.max_price_levels
    // );
    m_price_level_pool = m_allocator.allocate(m_price_level_pool_size);
}

OrderBookAllocator::~OrderBookAllocator() noexcept {
    // try {
    //     reset();
    //     std::unordered_map<std::string, OrderNode*> empty_map;
    //     m_order_map.swap(empty_map);
        
    //     // Don't delete the pools - they're managed by AllocatorManager
    //     m_order_pool = nullptr;
    //     m_price_level_pool = nullptr;
    // } catch (...) {}
    // try {
    //     reset(); // Clean up all allocations
    //     m_order_pool = nullptr;
    //     m_price_level_pool = nullptr;
    //     std::cerr << "[~OrderBookAllocator] Destructor completed. Resources released.\n";
    // } catch (const std::exception& e) {
    //     std::cerr << "[~OrderBookAllocator] Exception during destruction: " << e.what() << "\n";
    // } catch (...) {
    //     std::cerr << "[~OrderBookAllocator] Unknown exception during destruction.\n";
    // }
    try {
        // First clean up all active orders and price levels
        reset();
        
        // Then clean up the memory pools
        // if (m_order_pool) {
        //     m_allocator.deallocate(m_order_pool, m_order_pool_size);
        //     m_order_pool = nullptr;
        // }
        
        // if (m_price_level_pool) {
        //     m_allocator.deallocate(m_price_level_pool, m_price_level_pool_size);
        //     m_price_level_pool = nullptr;
        // }
        m_order_pool = nullptr;
        m_price_level_pool = nullptr;
        
        std::cerr << "[~OrderBookAllocator] Cleanup completed successfully\n";
    } catch (const std::exception& e) {
        std::cerr << "[~OrderBookAllocator] Exception during cleanup: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[~OrderBookAllocator] Unknown exception during cleanup\n";
    }
}

OrderNode* OrderBookAllocator::allocateOrder() {
    // if (m_active_orders.load(std::memory_order_relaxed) >= m_config.max_orders) {
    //     return nullptr;
    // }

    // try {
    //     // Allocate memory for the order
    //     std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
    //     void* memory = m_allocator.allocate(order_size);
    //     if (!memory) return nullptr;

    //     // Properly construct the OrderNode
    //     OrderNode* node = new (memory) OrderNode();  // Placement new
        
    //     // Initialize all fields to safe defaults
    //     node->price = 0.0;
    //     node->quantity = 0.0;
    //     node->order_id.clear();
    //     node->next = nullptr;
    //     node->prev = nullptr;
    //     node->parent_level = nullptr;
        
    //     m_active_orders.fetch_add(1, std::memory_order_release);
    //     return node;
    // } catch (...) {
    //     return nullptr;
    // }
   if (m_active_orders.load(std::memory_order_relaxed) >= m_config.max_orders) {
        std::cerr << "[allocateOrder] Max orders reached. Allocation failed.\n";
        return nullptr;
    }

    try {
        std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;

        // Debug: Log memory allocation offset
        std::cerr << "[allocateOrder] Allocating order memory. Active orders: "
                  << m_active_orders.load(std::memory_order_relaxed) << "\n";

        void* memory = static_cast<char*>(m_order_pool) + 
                      (m_active_orders.load(std::memory_order_relaxed) * order_size);
        
        if (!memory) {
            std::cerr << "[allocateOrder] Allocation returned nullptr!\n";
            return nullptr;
        }

        // Debug: Placement new
        std::cerr << "[allocateOrder] Constructing new OrderNode at memory address "
                  << memory << "\n";

        OrderNode* node = new (memory) OrderNode();
        node->price = 0.0;
        node->quantity = 0.0;
        node->order_id.clear();
        node->next = nullptr;
        node->prev = nullptr;
        node->parent_level = nullptr;
        
        m_active_orders.fetch_add(1, std::memory_order_release);

        std::cerr << "[allocateOrder] OrderNode allocated successfully. Active orders: "
                  << m_active_orders.load(std::memory_order_relaxed) << "\n";

        return node;
    } catch (const std::exception& e) {
        std::cerr << "[allocateOrder] Exception during allocation: " << e.what() << "\n";
        return nullptr;
    } catch (...) {
        std::cerr << "[allocateOrder] Unknown exception during allocation.\n";
        return nullptr;
    }
}
void OrderBookAllocator::deallocateOrder(OrderNode* order) {
    // if (!order) return;

    // try {
    //     // Safely remove from lookup map first
    //     if (!order->order_id.empty()) {
    //       std::lock_guard<std::mutex> lock(m_order_map_mutex); //protect access  
    //       m_order_map.erase(order->order_id);
    //     }

    //     // Safely unlink from parent level
    //     if (order->parent_level) {
    //         if (order->parent_level->first_order == order) {
    //             order->parent_level->first_order = order->next;
    //         }
    //         if (order->parent_level->last_order == order) {
    //             order->parent_level->last_order = order->prev;
    //         }
    //         if (order->parent_level->order_count > 0) {
    //             order->parent_level->order_count--;
    //         }
    //         order->parent_level->total_quantity -= order->quantity;
    //     }

    //     // Safely update links
    //     if (order->prev) {
    //         order->prev->next = order->next;
    //     }
    //     if (order->next) {
    //         order->next->prev = order->prev;
    //     }

    //     // Clear the order's pointers before deallocation
    //     order->next = nullptr;
    //     order->prev = nullptr;
    //     order->parent_level = nullptr;

    //     // Finally deallocate
    //     std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
    //     m_allocator.deallocate(order, order_size);
        
    //     if (m_active_orders > 0) {
    //         m_active_orders--;
    //     }
    // }
    // catch (const std::exception& e) {
    //     std::cerr << "Error in deallocateOrder: " << e.what() << std::endl;
    //     throw;
    // }
    // if (!order) {
    //     std::cerr << "[deallocateOrder] Null order passed. Ignoring deallocation.\n";
    //     return;
    // }

    // try {
    //     std::cerr << "[deallocateOrder] Deallocating order with ID: " 
    //               << (order->order_id.empty() ? "(unknown)" : order->order_id) << "\n";

    //     // Remove from map
    //     if (!order->order_id.empty()) {
    //         std::lock_guard<std::mutex> lock(m_order_map_mutex);
    //         m_order_map.erase(order->order_id);
    //     }

    //     // Unlink from parent level
    //     if (order->parent_level) {
    //         if (order->parent_level->first_order == order) {
    //             order->parent_level->first_order = order->next;
    //         }
    //         if (order->parent_level->last_order == order) {
    //             order->parent_level->last_order = order->prev;
    //         }
    //         if (order->parent_level->order_count > 0) {
    //             order->parent_level->order_count--;
    //         }
    //         order->parent_level->total_quantity -= order->quantity;
    //     }

    //     // Safely unlink
    //     if (order->prev) {
    //         order->prev->next = order->next;
    //     }
    //     if (order->next) {
    //         order->next->prev = order->prev;
    //     }

    //     // Clear the order's pointers
    //     order->next = nullptr;
    //     order->prev = nullptr;
    //     order->parent_level = nullptr;

    //     // Deallocate memory
    //     std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
    //     m_allocator.deallocate(order, order_size);

    //     m_active_orders.fetch_sub(1, std::memory_order_release);

    //     std::cerr << "[deallocateOrder] OrderNode deallocated successfully. Active orders: "
    //               << m_active_orders.load(std::memory_order_relaxed) << "\n";
    // } catch (const std::exception& e) {
    //     std::cerr << "[deallocateOrder] Exception during deallocation: " << e.what() << "\n";
    // } catch (...) {
    //     std::cerr << "[deallocateOrder] Unknown exception during deallocation.\n";
    // }
    if (!order) {
        std::cerr << "[deallocateOrder] Null order passed. Ignoring deallocation.\n";
        return;
    }

    try {
        // Remove from the map
        order->order_id.clear();  // Release string memory
        order->order_id.shrink_to_fit();  // Release capacity
        if (!order->order_id.empty()) {
            std::lock_guard<std::mutex> lock(m_order_map_mutex);
            m_order_map.erase(order->order_id);
        }

        // Unlink from the parent level
        if (order->parent_level) {
            if (order->parent_level->first_order == order) {
                order->parent_level->first_order = order->next;
            }
            if (order->parent_level->last_order == order) {
                order->parent_level->last_order = order->prev;
            }
            if (order->parent_level->order_count > 0) {
                order->parent_level->order_count--;
            }
            order->parent_level->total_quantity -= order->quantity;
        }

        // Unlink from sibling orders
        if (order->prev) {
            order->prev->next = order->next;
        }
        if (order->next) {
            order->next->prev = order->prev;
        }

        // Clear dynamic fields
        order->order_id.clear();

        // Deallocate memory
        std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
        m_allocator.deallocate(order, order_size);
        m_active_orders.fetch_sub(1, std::memory_order_relaxed);

        std::cerr << "[deallocateOrder] OrderNode deallocated. Active orders: "
                  << m_active_orders.load(std::memory_order_relaxed) << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[deallocateOrder] Exception during deallocation: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[deallocateOrder] Unknown exception during deallocation.\n";
    }
}
void cleanup(OrderBookAllocator& allocator) {
    allocator.reset();  // This should now properly clean up all resources
}

PriceLevel* OrderBookAllocator::allocatePriceLevel() {
    if (m_active_price_levels.load(std::memory_order_relaxed) >= m_config.max_price_levels) {
        return nullptr;  // Pool exhausted
    }

    PriceLevel* level = reinterpret_cast<PriceLevel*>(
        m_allocator.allocate(sizeof(PriceLevel))
    );
    
    if (level) {
        // Initialize level
        level->first_order = nullptr;
        level->last_order = nullptr;
        level->next = nullptr;
        level->prev = nullptr;
        level->order_count = 0;
        level->total_quantity = 0.0;
        
        // Update statistics
        m_active_price_levels++;
    }
    
    return level;
}

void OrderBookAllocator::deallocatePriceLevel(PriceLevel* level) {
    // if (!level) return;

    // try {
    //     // First safely unlink and deallocate all orders
    //     while (level->first_order) {
    //         OrderNode* order = level->first_order;
    //         level->first_order = order->next;
            
    //         // Unregister if necessary
    //         if (!order->order_id.empty()) {
    //             m_order_map.erase(order->order_id);
    //         }
            
    //         // Clear order's links
    //         order->next = nullptr;
    //         order->prev = nullptr;
    //         order->parent_level = nullptr;
            
    //         // Deallocate order
    //         m_allocator.deallocate(order, sizeof(OrderNode));
            
    //         if (m_active_orders > 0) {
    //             m_active_orders--;
    //         }
    //     }

    //     // Clear level's pointers
    //     level->first_order = nullptr;
    //     level->last_order = nullptr;
    //     level->next = nullptr;
    //     level->prev = nullptr;
    //     level->order_count = 0;
    //     level->total_quantity = 0;

    //     // Finally deallocate the level itself
    //     m_allocator.deallocate(level, sizeof(PriceLevel));
        
    //     if (m_active_price_levels > 0) {
    //         m_active_price_levels--;
    //     }
    // }
    // catch (const std::exception& e) {
    //     std::cerr << "Error in deallocatePriceLevel: " << e.what() << std::endl;
    //     throw;
    // }
    if (!level) {
        std::cerr << "[deallocatePriceLevel] Null level passed. Ignoring deallocation.\n";
        return;
    }

    try {
        // Unlink and deallocate all orders associated with this price level
        while (level->first_order) {
            OrderNode* order = level->first_order;
            level->first_order = order->next;
            deallocateOrder(order);
        }

        // Clear the level's pointers
        level->first_order = nullptr;
        level->last_order = nullptr;
        level->next = nullptr;
        level->prev = nullptr;
        level->order_count = 0;
        level->total_quantity = 0.0;

        // Deallocate the level itself
        m_allocator.deallocate(level, sizeof(PriceLevel));
        m_active_price_levels.fetch_sub(1, std::memory_order_relaxed);

        std::cerr << "[deallocatePriceLevel] PriceLevel deallocated. Active price levels: "
                  << m_active_price_levels.load(std::memory_order_relaxed) << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[deallocatePriceLevel] Exception during deallocation: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[deallocatePriceLevel] Unknown exception during deallocation.\n";
    }
}

OrderNode* OrderBookAllocator::findOrder(const std::string& order_id) {
    std::lock_guard<std::mutex> lock(m_order_map_mutex); //Protect access
    auto it = m_order_map.find(order_id);
    return (it != m_order_map.end()) ? it->second : nullptr;
}

void OrderBookAllocator::registerOrder(const std::string& order_id, OrderNode* order) {
    if (order) {
        std::lock_guard<std::mutex> lock(m_order_map_mutex); //protect access
        order->order_id = order_id;
        m_order_map[order_id] = order;
    }
}

void OrderBookAllocator::unregisterOrder(const std::string& order_id) {   
  std::lock_guard<std::mutex> lock(m_order_map_mutex); // protect access
  m_order_map.erase(order_id);
}

void OrderBookAllocator::reset() {
    // Clear all allocations
    m_order_map.clear();
    
    // Reset statistics
    m_active_orders = 0;
    m_active_price_levels = 0;
    m_order_modifications = 0;
    m_peak_orders = 0;
    m_peak_memory = 0;
}

OrderBookAllocator::Stats OrderBookAllocator::getStats() const {
    return Stats{
        m_active_orders.load(),
        m_active_price_levels.load(),
        calculateTotalMemoryUsed(),
        m_config.max_orders - m_active_orders.load(),
        m_config.max_price_levels - m_active_price_levels.load(),
        m_order_modifications.load(),
        m_peak_orders.load(),
        m_peak_memory.load()
    };
}

std::size_t OrderBookAllocator::calculateTotalMemoryUsed() const {
    std::size_t order_memory = m_active_orders.load() * 
        (sizeof(OrderNode) + m_config.order_data_size);
    std::size_t level_memory = m_active_price_levels.load() * sizeof(PriceLevel);
    return order_memory + level_memory;
}

bool OrderBookAllocator::hasCapacity() const {
    return m_active_orders.load() < m_config.max_orders &&
           m_active_price_levels.load() < m_config.max_price_levels;
}

}}} // namespaces
