#include "../../../include/mercuryTrade/core/memory/mercTransactionAllocator.hpp"
#include <stdexcept>
#include <algorithm> 

namespace mercuryTrade{
    namespace core{
        namespace memory{
            transactionAllocator::transactionAllocator(const Config& config)
            : m_config(config)
            , m_transaction_pool(nullptr)
            , m_batch_pool(nullptr)
            {
                if (config.max_transactions == 0 || config.max_batches == 0 || config.batch_size == 0){
                    throw std::invalid_argument("Invalid transaction allocator configuration");
                }
                // Allocate transaction pools
                std::size_t transaction_size = sizeof(transactionNode) + config.transaction_data_size;
                m_transaction_pool = m_allocator.allocate(transaction_size * config.max_transactions);

                // Allocate batch pool
                m_batch_pool = m_allocator.allocate(sizeof(transactionBatch) * config.max_batches);
            }

            transactionAllocator::~transactionAllocator() noexcept{
                try{
                    reset();
                    if (m_transaction_pool){
                        std::size_t transaction_size = sizeof(transactionNode) + m_config.transaction_data_size;
                        m_allocator.deallocate(m_transaction_pool,transaction_size * m_config.max_transactions);
                    }
                    if (m_batch_pool){
                        m_allocator.deallocate(m_batch_pool,sizeof(transactionBatch) * m_config.max_batches);
                    }
                } catch(...){}
            }

            transactionNode* transactionAllocator::beginTransaction(){
                if (m_active_transactions.load(std::memory_order_relaxed) >= m_config.max_transactions) return nullptr;
                transactionNode* node = nullptr;
                transactionBatch* batch = nullptr;

                // First we try to find a batch where there is some space
                for (auto* current_batch : m_active_batch_list){
                    if (current_batch && current_batch->used < current_batch->capacity){
                        batch = current_batch;
                        break;
                    }
                }
                // If no batch found with space, allocate new batch
                if (!batch){
                    batch = allocateBatch();
                    if (!batch) return nullptr;
                }
                // Allocate transaction node
                std::size_t transaction_size = sizeof(transactionNode) + m_config.transaction_data_size;
                void* memory = m_allocator.allocate(transaction_size);
                if (!memory) return nullptr;

                node = new (memory) transactionNode();
                node -> status = transactionStatus::PENDING;
                node -> parent_batch = batch;
                node -> next = nullptr;
                node -> prev = nullptr;
                node -> data_size = m_config.transaction_data_size;

                // Link to batch
                if (batch -> used == 0){
                    batch -> first_transaction = node;
                } else {
                    node -> prev = batch -> last_transaction;
                    batch -> last_transaction -> next = node; 
                }
                batch -> last_transaction = node;
                batch -> used ++;
                m_active_transactions++;
                m_peak_transactions = std::max(m_peak_transactions.load(),m_active_transactions.load());
                return node;
            }

            bool transactionAllocator::commitTransaction(transactionNode* transaction){
                if (!validateTransaction(transaction)) return false;
                if (transaction -> status != transactionStatus::PENDING) return false;
                transaction -> status = transactionStatus::COMMITTED;
                return true;
            }

            bool transactionAllocator::rollbackTransaction(transactionNode* transaction){
                if (!validateTransaction(transaction)) return false;
                if (transaction -> status != transactionStatus::PENDING) return false;
                transaction -> status = transactionStatus::ROLLED_BACK;
                m_rollbacks++;
                return true;
            }
            void transactionAllocator::endTransaction(transactionNode* transaction){
                if (!transaction) return;
                // Remove from lookup
                m_transaction_map.erase(transaction -> transaction_id);
                // Unlink from batch
                transactionBatch* batch = transaction -> parent_batch;
                if (batch){
                    if (batch -> first_transaction == transaction){
                        batch -> first_transaction == transaction->next;
                    }
                    if (batch -> last_transaction == transaction){
                        batch -> last_transaction == transaction -> prev;
                    }
                    batch -> used--;
                    // If batch empty then we deallocate it
                    if (batch -> used == 0){
                        deallocateBatch(batch);
                    }
                }
                // Update the links
                if (transaction -> prev) transaction -> prev -> next = transaction -> next;
                if (transaction -> next) transaction -> next -> prev = transaction -> prev;
                // Deallocate the transaction
                std::size_t transaction_size = sizeof(transactionNode) + m_config.transaction_data_size;
                m_allocator.deallocate(transaction,transaction_size);
                m_active_transactions--;
            }
            transactionBatch* transactionAllocator::allocateBatch(){
                if (m_active_batches.load(std::memory_order_relaxed) >= m_config.max_batches){
                    return nullptr;
                }
                // Allocate a new batch
                transactionBatch* batch = reinterpret_cast<transactionBatch*>(m_allocator.allocate(sizeof(transactionBatch)));
                if (!batch) return nullptr;
                // Initialize batch
                batch -> capacity = m_config.batch_size;
                batch -> used = 0;
                batch -> first_transaction = nullptr;
                batch -> last_transaction = nullptr;
                batch -> next = nullptr;
                batch -> prev = nullptr;
                batch -> is_active = true;

                // Add to the active list
                m_active_batch_list.push_back(batch);
                m_active_batches++;
                m_batch_allocs++;
                return batch;
            }
            void transactionAllocator::deallocateBatch(transactionBatch* batch) {
            if (!batch) return;

            try {
                // First safely end all transactions in the batch
                transactionNode* current = batch->first_transaction;
                while (current) {
                    transactionNode* next = current->next;
                    if (!current->transaction_id.empty()) {
                        unregisterTransaction(current->transaction_id);
                     }
                    // Store next before potentially invalidating current
                    current = next;
                }

                // Remove from active list
                /*auto it = std::find(m_active_batch_list.begin(), 
                m_active_batch_list.end(), batch);
                if (it != m_active_batch_list.end()) {
                    m_active_batch_list.erase(it);
                }*/
                
                m_active_batch_list.erase(
                    std::remove(m_active_batch_list.begin(),
                                m_active_batch_list.end(),
                                batch),
                    m_active_batch_list.end()
                );

                // Clear batch data
                batch->first_transaction = nullptr;
                batch->last_transaction = nullptr;
                batch->used = 0;
                batch->is_active = false;

                m_allocator.deallocate(batch, sizeof(transactionBatch));
                m_active_batches--;
            }
            catch (const std::exception& e) {
                std::cerr << "Error in deallocateBatch: " << e.what() << std::endl;
                throw;
            }
        }
            transactionNode* transactionAllocator::findTransaction(const std::string& transaction_id){
                auto it = m_transaction_map.find(transaction_id);
                return (it != m_transaction_map.end()) ? it -> second : nullptr;
            }
            void transactionAllocator::registerTransaction(const std::string &transaction_id, transactionNode* transaction){
                if (transaction){
                    transaction -> transaction_id = transaction_id;
                    m_transaction_map[transaction_id] = transaction;
                }
            }
            void transactionAllocator::unregisterTransaction(const std::string& transaction_id){
                m_transaction_map.erase(transaction_id);
            }

            transactionAllocator::Stats transactionAllocator::getStats() const{
                return Stats{
                    m_active_transactions.load(),
                    m_active_batches.load(),
                    calculateTotalMemoryUsed(),
                    m_peak_transactions.load(),
                    m_rollbacks.load(),
                    m_batch_allocs.load(),
                    calculateBatchUtilization()
                };
            }
            void transactionAllocator::reset(){
                // Clean up all batches
                for (auto* batch : m_active_batch_list){
                    if (batch){
                        cleanupBatch(batch);
                        m_allocator.deallocate(batch,sizeof(transactionBatch));
                    }
                }
                m_active_batch_list.clear();
                // Reset all counters
                m_active_transactions = 0;
                m_active_batches = 0;
                m_peak_transactions = 0;
                m_rollbacks = 0;
                m_batch_allocs = 0;
                m_transaction_map.clear();
            }
            bool transactionAllocator::hasCapacity() const{
                return m_active_transactions.load() < m_config.max_transactions && m_active_batches.load() < m_config.max_batches;
            }
            void transactionAllocator::cleanupBatch(transactionBatch* batch){
                if (!batch) return;
                transactionNode* current = batch -> first_transaction;
                while(current){
                    transactionNode* next = current -> next;
                    endTransaction(current);
                    current = next;
                }
            }
            bool transactionAllocator::validateTransaction(const transactionNode* transaction) const{
                if (!transaction) return false;
                if (!transaction -> parent_batch) return false;
                // Verify transaction belongs to a valid batch
                auto it = std::find(m_active_batch_list.begin(),m_active_batch_list.end(),transaction->parent_batch);
                return it != m_active_batch_list.end();
            }
            std::size_t transactionAllocator::calculateTotalMemoryUsed() const{
                std::size_t transaction_memory = m_active_transactions.load() * (sizeof(transactionNode) + m_config.transaction_data_size);
                std::size_t batch_memory = m_active_batches.load() * sizeof(transactionBatch);
                return transaction_memory + batch_memory;
            }
            double transactionAllocator::calculateBatchUtilization() const{
                if (m_active_batches.load() == 0) return 0.0;
                std::size_t total_used = 0;
                std::size_t total_capacity = 0;
                for (const auto* batch : m_active_batch_list){
                    if (batch){
                        total_used += batch -> used;
                        total_capacity += batch -> capacity;
                    }
                }
                return total_capacity > 0 ? static_cast<double>(total_used)/total_capacity : 0.0;
            }
        }
    }
}
