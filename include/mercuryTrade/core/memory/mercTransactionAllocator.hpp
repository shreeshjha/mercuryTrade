#ifndef MERC_TRANSACTION_ALLOCATOR_HPP
#define MERC_TRANSACTION_ALLOCATOR_HPP

#include "mercAllocatorManager.hpp"
#include <atomic>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace mercuryTrade{
    namespace core{
        namespace memory{
            // Forward Declarations
            struct transactionNode;
            struct transactionBatch;

            class transactionAllocator{
                public:
                    // The configuration for transaction memory management
                    struct Config{
                        std::size_t max_transactions; // The maximum number of active transactions
                        std::size_t max_batches; // The maximum number of batches
                        std::size_t batch_size; // Number of transactions per batch
                        std::size_t transaction_data_size; // Size of additional transaction data
                        bool enable_rollback; // Whether to support transaction rollback

                        static Config getDefaultConfig(){
                            return Config{
                                1000000, // max_transactions
                                1000, //max_batches
                                1000, //batch_size
                                256, //data_size
                                true //rollback is true by default
                            };
                        }
                    };

                    // Statistics about transaction memory usage
                    struct Stats{
                        std::size_t active_transactions;
                        std::size_t active_batches;
                        std::size_t total_memory_used;
                        std::size_t peak_transactions;
                        std::size_t rollbacks_performed;
                        std::size_t batch_allocations;
                        double average_batch_utilization;
                    };

                    // Transaction Status Tracking
                    enum class transactionStatus{
                        PENDING,
                        COMMITTED,
                        ROLLED_BACK,
                        FAILED
                    };

                    explicit transactionAllocator(const Config& config = Config::getDefaultConfig());
                    ~transactionAllocator() noexcept;

                    // Prevent copying
                    transactionAllocator(const transactionAllocator&) = delete;
                    transactionAllocator& operator = (const transactionAllocator&) = delete;

                    // Allow moving
                    transactionAllocator(transactionAllocator&&) noexcept;
                    transactionAllocator& operator = (const transactionAllocator&) = delete;

                    // Core transaction methods
                    transactionNode* beginTransaction();
                    bool commitTransaction(transactionNode* transaction);
                    bool rollbackTransaction(transactionNode* transaction);
                    void endTransaction(transactionNode* transaction);

                    // Batch Management
                    transactionBatch* allocateBatch();
                    void deallocateBatch(transactionBatch* batch);

                    // Transaction lookup and tracking
                    transactionNode* findTransaction(const std::string& transaction_id);
                    void registerTransaction(const std::string& transaction_id, transactionNode* transaction);
                    void unregisterTransaction(const std::string &transaction_id);

                    // Utility methods
                    Stats getStats() const;
                    void reset();
                    bool hasCapacity() const;

                private: 
                    AllocatorManager m_allocator;
                    Config m_config;
                    // Memory pools
                    void* m_transaction_pool;
                    void* m_batch_pool;
                    // Statistics tracking
                    std::atomic<std::size_t> m_active_transactions{0};
                    std::atomic<std::size_t> m_active_batches{0};
                    std::atomic<std::size_t> m_peak_transactions{0};
                    std::atomic<std::size_t> m_rollbacks{0};
                    std::atomic<std::size_t> m_batch_allocs{0};

                    // Transaction tracking
                    std::unordered_map<std::string,transactionNode*> m_transaction_map;
                    std::vector<transactionBatch*> m_active_batch_list;

                    // Helper methods
                    void cleanupBatch(transactionBatch* batch);
                    bool validateTransaction(const transactionNode* transaction) const;
                    std::size_t calculateTotalMemoryUsed() const;
                    double calculateBatchUtilization() const;    
            };
            // Transaction data structure
            struct transactionNode{
                std::string transaction_id;
                transactionAllocator::transactionStatus status;
                transactionBatch* parent_batch;
                transactionNode* next;
                transactionNode* prev;
                std::size_t data_size;
                char data[];
            };

            struct transactionBatch{
                std::size_t capacity;
                std::size_t used;
                transactionNode* first_transaction;
                transactionNode* last_transaction;
                transactionBatch* next;
                transactionBatch* prev;
                bool is_active;
            };
        }
    }
}


#endif