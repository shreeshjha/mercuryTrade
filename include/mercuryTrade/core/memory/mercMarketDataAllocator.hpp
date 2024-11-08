#ifndef MERC_MARKET_DATA_ALLOCATOR_HPP
#define MERC_MARKET_DATA_ALLOCATOR_HPP

#include "mercAllocatorManager.hpp"
#include <atomic>
#include <cstddef>

namespace mercuryTrade{
    namespace core{
        namespace memory{
            class marketDataAllocator{
                public:
                    struct bufferConfig{
                        std::size_t quote_size; // size for quote messages
                        std::size_t trade_size; // size for trade messages
                        std::size_t snapshot_size; // size for market snapshot
                        std::size_t buffer_capacity; // Number of messages per buffer
                    };
                private:
                    AllocatorManager m_allocator;
                    bufferConfig m_config;
                    std::atomic<std::size_t> m_quotes_allocated{0};
                    std::atomic<std::size_t> m_trades_allocated{0};
                    std::atomic<std::size_t> m_snapshots_allocated{0};
                public:
                    // initialize with specific buffer sizes
                    explicit marketDataAllocator(const bufferConfig& config = getDefaultConfig());

                    // Prevent copying
                    marketDataAllocator(const marketDataAllocator&) = delete;
                    marketDataAllocator& operator = (const marketDataAllocator&) = delete;

                    // Allow moving
                    marketDataAllocator(marketDataAllocator&&) noexcept = default;
                    marketDataAllocator& operator = (marketDataAllocator&&) noexcept = default;

                    // Allocation methods for different market data types
                    void* allocateQuoteBuffer();
                    void* allocateTradeBuffer();
                    void* allocateSnapshotBuffer();

                    void deallocateBuffer(void* ptr, std::size_t size);

                    // statistics and monitoring
                    struct allocationStats{
                        std::size_t quotes_allocated;
                        std::size_t trades_allocated;
                        std::size_t snapshots_allocated;
                        std::size_t total_memory_used;
                    };

                    allocationStats getStats() const;

                    // Default config
                    static bufferConfig getDefaultConfig(){
                        return bufferConfig{
                            64, // quote size
                            48, // trade size
                            1024, // snapshot size
                            1000 // buffer capacity
                        };
                    }
            };
        }
    }
}

#endif