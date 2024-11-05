#ifndef MERC_ALLOCATOR_MANAGER_HPP
#define MERC_ALLOCATOR_MANAGER_HPP

#include "mercAllocator.hpp"
#include <vector>
#include <memory>
#include <mutex>

namespace mercuryTrade {
  namespace core {
    namespace memory {

class AllocatorManager {
private:
  struct PoolInfo {
    std::size_t block_size;
    std::unique_ptr<FixedAllocator> allocator;

    PoolInfo(std::size_t size, std::size_t pool_size) 
      : block_size(size)
      , allocator(std::make_unique<FixedAllocator>(pool_size)) {}
  };

  // Predefined block sizes (powers of 2 for simplicity)
  static constexpr std::size_t MIN_BLOCK_SIZE = 8; // Minimum block size
  static constexpr std::size_t MAX_BLOCK_SIZE = 4096; // Maximum block size for pooling
  static constexpr std::size_t DEFAULT_POOL_SIZE = 1024; // Defaults blocks per pool 

  std::vector<PoolInfo> m_pools; // Vector of memory pools
  mutable std::mutex m_mutex; // Mutex for thread safety 

  // Helper methods
  std::size_t findPoolIndex(std::size_t size) const;
  std::size_t roundUpToNextPowerOf2(std::size_t size) const;

  void initializePools();

public:
  AllocatorManager();
  ~AllocatorManager() noexcept = default;

  //Prevent copying 
  AllocatorManager(const AllocatorManager&) = delete;
  AllocatorManager& operator = (const AllocatorManager&) = delete;

  //Allow moving
  AllocatorManager(AllocatorManager&&) noexcept = default;
  AllocatorManager& operator = (AllocatorManager&&) noexcept = default;   
  //Core allocation methods
  void* allocate(std::size_t size);
  void deallocate(void* ptr, std::size_t size);

  //Utility methods
  std::size_t getPoolCount() const noexcept;
  std::size_t getTotalMemoryUsed() const noexcept;
  std::size_t getBlocksInUse(std::size_t blockSize) const;
  bool isPoolAvailable(std::size_t size) const noexcept;

  //Statistics
  struct PoolStats {
    std::size_t block_size;
    std::size_t blocks_in_use;
    std::size_t total_blocks;
    std::size_t memory_used;
  };

  std::vector<PoolStats> getPoolStats() const;
};

    }
  }
}

#endif // MERC_ALLOCATOR_MANAGER_HPP
