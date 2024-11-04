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
        : m_free_list(nullptr)
        , m_pool(std::make_unique<memoryBlock[]>(pool_size))
        , m_pool_size(pool_size)
        , m_blocks_in_use(0) 
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
        m_free_list.store(&m_pool[0], std::memory_order_release);
      }
      
      // Move Constructor
      FixedAllocator::FixedAllocator(FixedAllocator&& other) noexcept
        : m_free_list(other.m_free_list.load(std::memory_order_acquire))
        , m_pool(std::move(other.m_pool))
        , m_pool_size(other.m_pool_size)
        , m_blocks_in_use(other.m_blocks_in_use.load(std::memory_order_release))
      {
        // Reset the state of other` 
        other.m_free_list.store(nullptr,std::memory_order_release);
        other.m_pool_size = 0;
        other.m_blocks_in_use.store(0,std::memory_order_release);
      }

      // Move assignment operator
      FixedAllocator& FixedAllocator::operator=(FixedAllocator&& other) noexcept{
        if (this != &other){
          // Store the current state of other
          auto free_list = other.m_free_list.load(std::memory_order_acquire);
          auto blocks_in_use = other.m_blocks_in_use.load(std::memory_order_acquire);
          
          // Move the resources
          m_pool = std::move(other.m_pool);
          m_pool_size = other.m_pool_size;
          m_free_list.store(free_list,std::memory_order_release);
          m_blocks_in_use.store(blocks_in_use,std::memory_order_release);

          // Reset the state of other
          other.m_free_list.store(nullptr,std::memory_order_release);
          other.m_pool_size = 0;
          other.m_blocks_in_use.store(0,std::memory_order_release);
        }
        return *this;
      }

      //Destructor
      FixedAllocator:: ~FixedAllocator() noexcept{
        // Since m_pool is a unique_ptr, it will automatically
        // deallocate the memory when destroyed
      }

      // Implementation of utility methods
      std::size_t FixedAllocator::blocks_in_use() const noexcept{
        return m_blocks_in_use.load(std::memory_order_acquire);
      }

      std::size_t FixedAllocator::available_blocks() const noexcept{
        return m_pool_size - m_blocks_in_use.load(std::memory_order_acquire);
      }

    }
  }
}

int main() {
  std::cout << "Hello Sam, Start Contributing from here on";
  return 0;
}
