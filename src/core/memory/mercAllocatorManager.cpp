#include "../../../include/mercuryTrade/core/memory/mercAllocatorManager.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace mercuryTrade {
  namespace core {
    namespace memory {
      
// Helper method to round up to next power of 2
std::size_t AllocatorManager::roundUpToNextPowerOf2(std::size_t size) const {
  if(size == 0) return MIN_BLOCK_SIZE;

  --size;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size |= size >> 32;
  ++size;

  return size;
}

// Helper method to find the appropriate pool index for a given size

std::size_t AllocatorManager::findPoolIndex(std::size_t size) const {
  size = roundUpToNextPowerOf2(size);
  if(size < MIN_BLOCK_SIZE || size > MAX_BLOCK_SIZE) {
    throw std::out_of_range("Requested size outside supported range");
  }
  return static_cast<std::size_t>(std::log2(size) - std::log2(MIN_BLOCK_SIZE));
}

// Initialize pools with predefined block size 
void AllocatorManager::initializePools() {
  for(std::size_t size = MIN_BLOCK_SIZE; size <= MAX_BLOCK_SIZE; size*=2) {
    m_pools.emplace_back(size, DEFAULT_POOL_SIZE);
  }
}

//Constructor
AllocatorManager::AllocatorManager() {
  initializePools();
}

//Core allocation method 
void * AllocatorManager::allocate(std::size_t size) {
  if(size > MAX_BLOCK_SIZE) {
    //For large allocations, use system allocator 
    return ::operator new(size, std::nothrow);
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  std::size_t poolIndex = findPoolIndex(size);

  if(poolIndex >= m_pools.size()) {
    throw std::runtime_error("Invalid pool index");
  }

  void *ptr = m_pools[poolIndex].allocator->allocate();
  if(!ptr) {
    throw std::bad_alloc();
  }

  return ptr;
}

// Core deallocation method
void AllocatorManager::deallocate(void *ptr, std::size_t size) {
  if(!ptr) return;

  if(size > MAX_BLOCK_SIZE) {
    //For large allocations, use sysyem deallocator
    ::operator delete(ptr, std::nothrow);
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  std::size_t poolIndex = findPoolIndex(size);

  if(poolIndex >= m_pools.size()) {
    throw std::runtime_error("Invalid pool index");
  }

  m_pools[poolIndex].allocator->deallocate(ptr);
}

//Utility Methods
std::size_t AllocatorManager::getPoolCount() const noexcept {
    return m_pools.size();
}

std::size_t AllocatorManager::getTotalMemoryUsed() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::size_t total = 0;
    
    for (const auto& pool : m_pools) {
        total += pool.block_size * pool.allocator->blocks_in_use();
    }
    
    return total;
}

std::size_t AllocatorManager::getBlocksInUse(std::size_t blockSize) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    blockSize = roundUpToNextPowerOf2(blockSize);
    
    for (const auto& pool : m_pools) {
        if (pool.block_size == blockSize) {
            return pool.allocator->blocks_in_use();
        }
    }
    
    return 0;
}

bool AllocatorManager::isPoolAvailable(std::size_t size) const noexcept {
    try {
        size = roundUpToNextPowerOf2(size);
        return size >= MIN_BLOCK_SIZE && size <= MAX_BLOCK_SIZE;
    } catch (...) {
        return false;
    }
}

// Statistics method
std::vector<AllocatorManager::PoolStats> AllocatorManager::getPoolStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PoolStats> stats;
    stats.reserve(m_pools.size());
    
    for (const auto& pool : m_pools) {
        PoolStats poolStats{
            pool.block_size,
            pool.allocator->blocks_in_use(),
            pool.allocator->blocks_in_use() + pool.allocator->available_blocks(),
            pool.block_size * pool.allocator->blocks_in_use()
        };
        stats.push_back(poolStats);
    }
    
    return stats;
}
    }
  }
}
