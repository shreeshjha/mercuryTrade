#include "../../../include/mercuryTrade/core/memory/mercOrderBookAllocator.hpp"
#include <mutex>
#include <stdexcept>

namespace mercuryTrade {
namespace core {
namespace memory {

OrderBookAllocator::OrderBookAllocator(const Config& config)
    : m_config(config)
{
    if (config.max_orders == 0 || config.max_price_levels == 0) {
        throw std::invalid_argument("Invalid order book configuration");
    }

    // Allocate order pool
    std::size_t order_size = sizeof(OrderNode) + config.order_data_size;
    m_order_pool = m_allocator.allocate(order_size * config.max_orders);

    // Allocate price level pool
    m_price_level_pool = m_allocator.allocate(
        sizeof(PriceLevel) * config.max_price_levels
    );
}

OrderBookAllocator::~OrderBookAllocator() noexcept {
 try {
         std::cout << "[OrderBookAllocator] Starting cleanup..." << std::endl;

        // Ensure all allocated orders and price levels are cleaned up
        reset();

        // Deallocate the memory pools if not null
        if (m_order_pool) {
            std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
            m_allocator.deallocate(m_order_pool, order_size * m_config.max_orders);
            m_order_pool = nullptr;
        }

        if (m_price_level_pool) {
            m_allocator.deallocate(m_price_level_pool, sizeof(PriceLevel) * m_config.max_price_levels);
            m_price_level_pool = nullptr;
        }
        cleanup();

        std::cout << "[OrderBookAllocator] Cleanup completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[OrderBookAllocator] Exception during cleanup: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[OrderBookAllocator] Unknown error during cleanup" << std::endl;
    }
}

OrderNode* OrderBookAllocator::allocateOrder() {
    if (m_active_orders.load(std::memory_order_relaxed) >= m_config.max_orders) {
        return nullptr;
    }

    try {
        // Use the pre-allocated pool instead of allocating new memory
        std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
        void* memory = static_cast<char*>(m_order_pool) + 
                      (m_active_orders.load(std::memory_order_relaxed) * order_size);
        
        OrderNode* node = new (memory) OrderNode();
        node->price = 0.0;
        node->quantity = 0.0;
        node->order_id.clear();
        node->next = nullptr;
        node->prev = nullptr;
        node->parent_level = nullptr;
        
        m_active_orders.fetch_add(1, std::memory_order_release);
        return node;
    } catch (...) {
        return nullptr;
    }
}
void OrderBookAllocator::deallocateOrder(OrderNode* order) {
    if (!order) return;

    try {
        // Safely remove from lookup map first
        if (!order->order_id.empty()) {
          std::lock_guard<std::mutex> lock(m_order_map_mutex); //protect access  
          m_order_map.erase(order->order_id);
        }

        // Safely unlink from parent level
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

        // Safely update links
        if (order->prev) {
            order->prev->next = order->next;
        }
        if (order->next) {
            order->next->prev = order->prev;
        }

        // Clear the order's pointers before deallocation
        order->next = nullptr;
        order->prev = nullptr;
        order->parent_level = nullptr;

        // Finally deallocate
        std::size_t order_size = sizeof(OrderNode) + m_config.order_data_size;
        m_allocator.deallocate(order, order_size);
        
        if (m_active_orders > 0) {
            m_active_orders--;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in deallocateOrder: " << e.what() << std::endl;
        throw;
    }
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

        {
            // Track allocated level
            std::lock_guard<std::mutex> lock(m_tracking_mutex);
            m_allocated_price_levels.insert(level);
        }
        
        // Update statistics
        m_active_price_levels++;
    }
    
    return level;
}

void OrderBookAllocator::deallocatePriceLevel(PriceLevel* level) {
    if (!level) return;

    try {
        // First safely unlink and deallocate all orders
        while (level->first_order) {
            OrderNode* order = level->first_order;
            level->first_order = order->next;
            
            // Unregister if necessary
            if (!order->order_id.empty()) {
                m_order_map.erase(order->order_id);
            }
            
            // Clear order's links
            order->next = nullptr;
            order->prev = nullptr;
            order->parent_level = nullptr;
            
            // Deallocate order
            m_allocator.deallocate(order, sizeof(OrderNode));
            
            if (m_active_orders > 0) {
                m_active_orders--;
            }
        }

        // Clear level's pointers
        level->first_order = nullptr;
        level->last_order = nullptr;
        level->next = nullptr;
        level->prev = nullptr;
        level->order_count = 0;
        level->total_quantity = 0;
 
        // Finally deallocate the level itself
        m_allocator.deallocate(level, sizeof(PriceLevel));
        
        if (m_active_price_levels > 0) {
            m_active_price_levels--;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in deallocatePriceLevel: " << e.what() << std::endl;
        throw;
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

// void OrderBookAllocator::reset() {
//     try {
//         // First safely clear all order data
//         {
//             std::lock_guard<std::mutex> lock(m_order_map_mutex);
//             for (auto& pair : m_order_map) {
//                 if (pair.second) {
//                     // Clear string data
//                     pair.second->order_id.clear();
//                    pair.second->order_id.shrink_to_fit();
                    
//                     // Clear pointers
//                     pair.second->next = nullptr;
//                     pair.second->prev = nullptr;
//                     pair.second->parent_level = nullptr;
//                 }
//             }
//             m_order_map.clear();
//         }

//         // Reset statistics
//         m_active_orders.store(0);
//         m_active_price_levels.store(0);
//         m_order_modifications.store(0); // Fixed this from m_active_modifications
//         m_peak_orders.store(0);
//         m_peak_memory.store(0);
        
//         std::cout << "[OrderBookAllocator::reset] Successfully reset allocator state" << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "[OrderBookAllocator::reset] Error during reset: " << e.what() << std::endl;
//     } catch (...) {
//         std::cerr << "[OrderBookAllocator::reset] Unknown error during reset" << std::endl;
//     }
// }

void OrderBookAllocator::reset() {
    try {
        std::cout << "[OrderBookAllocator::reset] Starting reset process..." << std::endl;

        {
            std::lock_guard<std::mutex> lock(m_order_map_mutex);


            for (auto& pair : m_order_map) {
                if (pair.second) {
                
                    // Clear string data
                    pair.second->order_id.clear();
                    pair.second->order_id.shrink_to_fit();

                    // Clear pointers
                    pair.second->next = nullptr;
                    pair.second->prev = nullptr;
                    pair.second->parent_level = nullptr;
                }
            }
            m_order_map.clear();
        }


        for (PriceLevel* level : m_allocated_price_levels) {
            deallocatePriceLevel(level);
        }
        m_allocated_price_levels.clear();


        // Reset statistics
        m_active_orders.store(0, std::memory_order_release);
        m_active_price_levels.store(0, std::memory_order_release);
        m_order_modifications.store(0);
        m_peak_orders.store(0);
        m_peak_memory.store(0);

        std::cout << "[OrderBookAllocator::reset] Successfully reset allocator state" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[OrderBookAllocator::reset] Error during reset: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[OrderBookAllocator::reset] Unknown error during reset" << std::endl;
    }
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

void OrderBookAllocator::cleanup() {
    std::lock_guard<std::mutex> lock(m_tracking_mutex);
    for (PriceLevel* level : m_allocated_price_levels) {
        m_allocator.deallocate(level, sizeof(PriceLevel)); // Deallocate memory
    }
    m_allocated_price_levels.clear(); // Clear the set
    m_active_price_levels.store(0, std::memory_order_relaxed); // Reset count
}


}}} 

