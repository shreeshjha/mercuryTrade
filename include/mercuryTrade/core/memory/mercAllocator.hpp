#ifndef MERCURY_ALLOCATOR_HPP
#define MERCURY_ALLOCATOR_HPP

#include <iostream>
#include <stdio.h>
#include <cstddef>
#include <cstdlib>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <new>
#include <array>
#include <cassert>

namespace mercuryTrade {
  namespace core {
    namespace memory {
      // Constants (for namespace scope)
      namespace {
        constexpr std::size_t CACHE_LINE_SIZE = 64; // Cache for Modern CPU Line
        constexpr std::size_t DEFAULT_POOL_SIZE = 1024; // No of Blocks (1 kb each)
        constexpr std::size_t BLOCK_SIZE = 256; // Size of actual data in each Blocks
      }

      struct memoryBlock {
        // This is for memory alignment for cache line mentioned above
        alignas(CACHE_LINE_SIZE) struct {
          std::atomic<memoryBlock*> next; // This is the next available block in free list
          std::atomic<bool> is_allocated; // This tells whether the block is free or not
          
          //To avoid false sharing by multiple threads basically avoiding memory overlap
          char padding[CACHE_LINE_SIZE - sizeof(std::atomic<memoryBlock*>) - sizeof(std::atomic<bool>)];
        } header;
        
        // This is actual storage of data    
        alignas(CACHE_LINE_SIZE) std::byte data[BLOCK_SIZE];
      };

      class FixedAllocator {
        private:
          std::atomic<memoryBlock*> m_free_list; //This is a pointer to the first free block
          std::unique_ptr<memoryBlock[]> m_pool; //This is the memory pool containing the blocks
          std::size_t m_pool_size; //This is the current pool size or the total number of blocks
          std::atomic<std::size_t> m_blocks_in_use; //This tracks the number of allocated blocks;
        
        public:
          //The constructor takes the pool size
          explicit FixedAllocator(std::size_t pool_size = DEFAULT_POOL_SIZE);
          
          // Prevent Copying of allocators
          FixedAllocator(const FixedAllocator&) = delete;
          FixedAllocator& operator=(const FixedAllocator&) = delete;
          
          // This is to allow moving which is useful for initializing
          FixedAllocator(FixedAllocator&&) noexcept;
          FixedAllocator& operator=(FixedAllocator&&) noexcept;
          
          // Destructor
          ~FixedAllocator() noexcept;
          
          //These are the Core Allocation Methods
          void* allocate() noexcept;
          void deallocate(void* ptr) noexcept;
          
          //These are the utility methods
          std::size_t blocks_in_use() const noexcept;
          std::size_t available_blocks() const noexcept;
      };
    }
  }
}

#endif // MERCURY_ALLOCATOR_HPP

