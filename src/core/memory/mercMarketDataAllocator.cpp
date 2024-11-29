#include "../../../include/mercuryTrade/core/memory/mercMarketDataAllocator.hpp"
#include <stdexcept>


using namespace std::string_literals;

namespace mercuryTrade{
    namespace core{
        namespace memory{
            marketDataAllocator::marketDataAllocator(const bufferConfig &config) : m_config(config){
                // What happends when buffer configuration is invalid
                if (config.quote_size == 0 || config.trade_size == 0 || config.snapshot_size == 0 || config.buffer_capacity == 0){
                    throw std::runtime_error("Invalid buffer configuration"s);
                }
            }

            void* marketDataAllocator::allocateQuoteBuffer(){
                void* buffer = m_allocator.allocate(m_config.quote_size * m_config.buffer_capacity);
                if (buffer){
                    m_quotes_allocated.fetch_add(1,std::memory_order_relaxed);
                }
                return buffer;
            }

            void* marketDataAllocator::allocateTradeBuffer(){
                void* buffer = m_allocator.allocate(m_config.trade_size * m_config.buffer_capacity);
                if (buffer){
                    m_trades_allocated.fetch_add(1,std::memory_order_relaxed);
                }
                return buffer;
            }

            void* marketDataAllocator::allocateSnapshotBuffer(){
                void* buffer = m_allocator.allocate(m_config.snapshot_size * m_config.buffer_capacity);
                if (buffer){
                    m_snapshots_allocated.fetch_add(1,std::memory_order_relaxed);
                }
                return buffer;
            }

            void marketDataAllocator::deallocateBuffer(void* ptr,std::size_t size){
                if (!ptr) return;
                // Determine buffer type based on size and update statistics
                if (size == m_config.quote_size * m_config.buffer_capacity){
                    m_quotes_allocated.fetch_sub(1,std::memory_order_relaxed);
                } else if (size == m_config.trade_size * m_config.buffer_capacity){
                    m_trades_allocated.fetch_sub(1,std::memory_order_relaxed);
                } else if (size == m_config.snapshot_size * m_config.buffer_capacity){
                    m_snapshots_allocated.fetch_sub(1,std::memory_order_relaxed);
                }
                m_allocator.deallocate(ptr,size);
            }

            bool marketDataAllocator::hasCapacity() const noexcept {
                auto stats = getStats();
                return (stats.quotes_allocated < m_config.buffer_capacity &&
                        stats.trades_allocated < m_config.buffer_capacity &&
                        stats.snapshots_allocated < m_config.buffer_capacity);
            }

            marketDataAllocator::allocationStats marketDataAllocator::getStats() const{
                allocationStats stats;
                stats.quotes_allocated = m_quotes_allocated.load(std::memory_order_relaxed);
                stats.trades_allocated = m_trades_allocated.load(std::memory_order_relaxed);
                stats.snapshots_allocated = m_snapshots_allocated.load(std::memory_order_relaxed);

                // Calculate the total memory used
                stats.total_memory_used = (stats.quotes_allocated * m_config.quote_size * m_config.buffer_capacity) + 
                (stats.trades_allocated * m_config.trade_size * m_config.buffer_capacity) + 
                (stats.snapshots_allocated * m_config.snapshot_size * m_config.buffer_capacity);

                return stats;
            }
            
        }
    }
}
