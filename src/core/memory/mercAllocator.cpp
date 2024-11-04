/* Author(s): 
Shreesh Kumar Jha (MSc Computer Science and Engineering, Politecnico di Milano),
Samarth Bhatia (Msc Computer Science and Engineering, Politecnico di Milano)

@file allocator.cpp
@brief Implementation of custom fixed-size memory allocator for low-latency trading
*/

#include "../../../include/mercuryTrade/core/memory/mercAllocator.hpp"
// Namespace Structure for Our Application
namespace mercuryTrade {
  namespace core {
    namespace memory {
      // Constructor Implementation
      
      FixedAllocator::FixedAllocator(std::size_t pool_size) 
        : m_free_list(nullptr),
        , m_pool(std::make_unique<memoryBlock[]>(pool_size))
        , m_pool_size(pool_size)
        , m_block_in_use(0) 
      {
        // So we are initilising the free list by linking all blocks together
        for(std::size_t i = 0; i < pool_size - 1; i++) {
          m_pool[i].header.next.store(&m_pool[i+1], std::memory_order_release);
          m_pool[i].header.is_allocated.store(false, std::memory_order_release);
        }
        
        // Last block points to nullptr and initilises its allocation status
        m_pool[pool_size - 1].header.next.store(nullptr, std::memory_order_release);
        m_pool[pool_size - 1].header.is_allocated.store(false, std::memory_order_release);

        // Setting the head of our free list to the beginning of our pool       
        m_free_list.store(&m_pool[0] std::memory_order_release);
      }
      
      // Move Constructor

    }
  }
}

int main() {
  std::cout << "Hello Sam, Start Contributing from here on";
  return 0;
}
