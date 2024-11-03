/* Author(s): 
Shreesh Kumar Jha (MSc Computer Science and Engineering, Politecnico di Milano),
Samarth Bhatia (Msc Computer Science and Engineering, Politecnico di Milano)

@file allocator.cpp
@brief Implementation of custom fixed-size memory allocator for low-latency trading
*/

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
#include "mercuryTrade/include/core/memory/mercuryTrade.hpp"

// Namespace Structure for Our Application
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
        // This is for memory allignment for cache line mentioned above
        alignas(CACHE_LINE_SIZE) struct  {
         std::atomic<memoryBlock*> next; // This is the next available block in free list

         std::atomic<bool> is_allocated; // This tells whether the block is free or not
          
         //To avoid false sharing by multiple threads basically avoiding memory overlap
         char padding[CACHE_LINE_SIZE - sizeof(std::atomic<memoryBlock*>) - sizeof(std::atomic<bool>)];
        } header; // We are using header to group metadata about the block, avoids false sharing by padding the memoryBlock, also it means we are giving each block header its own cache line
         //
        // This is actual storage of data    
        alignas(CACHE_LINE_SIZE) std::byte data[BLOCK_SIZE]
      };
    }
  }
}

int main() {
  std::cout << "Hello Sam, Start Contributing from here on";
  return 0;
}
