#include "../../../include/mercuryTrade/core/memory/mercTradingManager.hpp"
#include <chrono>
#include <algorithm>
#include <stdexcept>
#include <thread>

namespace mercuryTrade{
    namespace core{
        namespace memory{
            tradingManager::tradingManager(const Config& config)
             : m_config(config)
             , m_status(Status::STARTING)
             , m_order_allocator(OrderBookAllocator::Config::getDefaultConfig())
             , m_market_data_allocator()
             , m_transaction_allocator(transactionAllocator::Config::getDefaultConfig())
             , m_metrics(std::make_unique<performanceMetrics>())
             , m_current_transaction(nullptr)             // Add this member variable
            {
                if (config.max_orders == 0 || config.max_symbols == 0) {
                    throw std::invalid_argument("Invalid trading configuration");
                }
                if (m_metrics){
                     m_metrics -> last_update = std::chrono::high_resolution_clock::now();
                    m_metrics -> order_count = 0;
                    m_metrics -> trade_count = 0;
                    m_metrics -> avg_latency = 0.0;
                }
            }

        tradingManager::~tradingManager() noexcept {
    try {
        if (m_status == Status::RUNNING) {
            stop();
        }
        
        // Clean up transactions first
        {
            std::lock_guard<std::mutex> tx_lock(m_thread_transactions_mutex);
            for (auto& pair : m_thread_transactions) {
                if (pair.second) {
                    m_transaction_allocator.rollbackTransaction(pair.second);
                    m_transaction_allocator.endTransaction(pair.second);
                }
            }
            m_thread_transactions.clear();
        }
        
        m_metrics.reset();
        cleanupResources();
        
        // Let the allocators clean up their own resources naturally
        // through their destructors
        
    } catch (...) {
        // Ensure no exceptions escape
    }
}



bool tradingManager::beginTransaction() {
    std::lock_guard<std::mutex> lock(m_transaction_mutex);
    
    if (m_status != Status::RUNNING) {
        return false;
    }
    
    try {
        auto thread_id = std::this_thread::get_id();
        
        // Check if thread already has a transaction
        {
            std::lock_guard<std::mutex> tx_lock(m_thread_transactions_mutex);
            if (m_thread_transactions.find(thread_id) != m_thread_transactions.end()) {
                return false;
            }
        }
        
        auto* transaction = m_transaction_allocator.beginTransaction();
        if (!transaction) {
            return false;
        }

        std::string transaction_id = "TX_" + std::to_string(m_pending_transactions.load()) + 
                                   "_" + std::to_string(std::hash<std::thread::id>{}(thread_id));
        
        {
            std::lock_guard<std::mutex> tx_lock(m_thread_transactions_mutex);
            m_transaction_allocator.registerTransaction(transaction_id, transaction);
            m_thread_transactions[thread_id] = static_cast<transactionNode*>(transaction);
        }
        
        m_pending_transactions++;
        return true;
    } catch (...) {
        return false;
    }
}

         bool tradingManager::commitTransaction() {
    std::lock_guard<std::mutex> lock(m_transaction_mutex);
    
    try {
        auto thread_id = std::this_thread::get_id();
        transactionNode* txNode = nullptr;
        
        {
            std::lock_guard<std::mutex> tx_lock(m_thread_transactions_mutex);
            auto it = m_thread_transactions.find(thread_id);
            if (it == m_thread_transactions.end()) {
                return false;
            }
            txNode = it->second;
            m_thread_transactions.erase(it);
        }
        
        if (!txNode || txNode->status != transactionAllocator::transactionStatus::PENDING) {
            return false;
        }

        bool success = m_transaction_allocator.commitTransaction(txNode);
        if (success) {
            m_transaction_allocator.endTransaction(txNode);
            if (m_pending_transactions > 0) {
                m_pending_transactions--;
            }
        }
        return success;
    } catch (...) {
        return false;
    }
}
 
            bool tradingManager::rollbackTransaction(){
                std::lock_guard<std::mutex> lock(m_transaction_mutex);
    
                try {
                  if (!m_current_transaction) {
                      return false;
                  }

                  auto* txNode = static_cast<transactionNode*>(m_current_transaction);
                  bool success = m_transaction_allocator.rollbackTransaction(txNode);
                  if (success) {
                    auto thread_id = std::this_thread::get_id();
                    m_thread_transactions.erase(thread_id);
                    m_transaction_allocator.endTransaction(txNode);
                    m_current_transaction = nullptr;
            
                    if (m_pending_transactions > 0) {
                      m_pending_transactions--;
                    }
                  }
                  return success;
                } catch (const std::exception& e) {
                    std::cout << "Exception in rollbackTransaction: " << e.what() << std::endl;
                  return false;
                }
            }

            void tradingManager::updateOrderBook(const std::string& symbol){
                if (symbol.empty()){
                    throw std::invalid_argument("Symbol cannot be empty");
                }
                try{
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time-start_time).count();
                    updateMetrics(static_cast<double>(latency));
                }catch(...){

                }
            }

            bool tradingManager::cancelOrder(const std::string& order_id){
                if (m_status != Status::RUNNING){
                    return false;
                }
                try{
                    auto start_time = std::chrono::high_resolution_clock::now();
                    if (m_config.enable_transactions){
                        beginTransaction();
                    }

                    if (m_config.enable_transactions){
                        commitTransaction();
                    }
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                    updateMetrics(static_cast<double>(latency));
                    return true;
                }catch (...){
                    if (m_config.enable_transactions){
                        rollbackTransaction();
                    }
                    return false;
                }
            }
            
            void tradingManager::optimizeMemory(){
                if (m_status != Status::RUNNING && m_status != Status::PAUSED){
                    return;
                }
                try{
                    auto initial_memory = calculateMemoryUsed();
                    m_order_allocator.hasCapacity();
                    m_market_data_allocator.hasCapacity();
                    m_transaction_allocator.hasCapacity();
                    auto final_memory = calculateMemoryUsed();
                    if (final_memory < initial_memory){
                        std::cout<< "Optimization success";
                    }
                }catch (...){

                }
            }
            
            // bool tradingManager::submitOrder(const order& ord){
            //     if (m_status != Status::RUNNING){
            //         return false;
            //     }
            //     if (!validateOrder(ord)){
            //         return false;
            //     }
            //     auto start_time = std::chrono::high_resolution_clock::now();
            //     try{
            //         // Begin a new transaction
            //         if (m_config.enable_transactions){
            //             if (!beginTransaction()) return false;
            //         }

            //         // Allocate order memory
            //         OrderNode* order_node = m_order_allocator.allocateOrder();
            //         if (!order_node){
            //             if (m_config.enable_transactions){
            //                 rollbackTransaction();
            //             }
            //             return false;
            //         }
            //         order_node -> price = ord.price;
            //         order_node -> quantity = ord.quantity;
            //         order_node -> order_id = ord.order_id;
            //         order_node -> parent_level = nullptr;
            //         order_node -> next = nullptr;
            //         order_node -> prev = nullptr;

                  
            //         try{
            //             // Update order book
            //             updateOrderBook(ord.symbol);
            //         }catch (...){
            //             m_order_allocator.deallocateOrder(order_node);
            //             if (m_config.enable_transactions){
            //                 rollbackTransaction();
            //             }
            //             return false;
            //         }

            //         // Update metrics
            //         m_active_orders++;
            //         auto end_time = std::chrono::high_resolution_clock::now();
            //         auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            //         updateMetrics(static_cast<double>(latency));

            //         if (m_config.enable_transactions){
            //             if (!commitTransaction()){
            //                 m_order_allocator.deallocateOrder(order_node);
            //                 return false;
            //             }
            //         }
            //         return true;
            //     } catch(...){
            //         if (m_config.enable_transactions){
            //             rollbackTransaction();
            //         }
            //         return false; 
            //     }
            // }
               bool tradingManager::submitOrder(const order& ord) {
    if (m_status != Status::RUNNING) {
        return false;
    }
    
    if (!validateOrder(ord)) {
        return false;
    }
    
    try {
        if (m_config.enable_transactions) {
            if (!beginTransaction()) {
                return false;
            }
        }
        
        OrderNode* order_node = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_order_mutex);
            order_node = m_order_allocator.allocateOrder();
        }
        
        if (!order_node) {
            if (m_config.enable_transactions) {
                rollbackTransaction();
            }
            return false;
        }

        order_node->price = ord.price;
        order_node->quantity = ord.quantity;
        m_order_allocator.registerOrder(ord.order_id, order_node);

        try {
            updateOrderBook(ord.symbol);
        } catch (...) {
            m_order_allocator.deallocateOrder(order_node);
            if (m_config.enable_transactions) {
                rollbackTransaction();
            }
            return false;
        }

        m_active_orders++;
        
        if (m_config.enable_transactions) {
            if (!commitTransaction()) {
                m_order_allocator.deallocateOrder(order_node);
                return false;
            }
        }
        return true;
    } catch (...) {
        if (m_config.enable_transactions) {
            rollbackTransaction();
        }
        return false;
    }
}        


    
              void tradingManager::handleMarketData(const marketData& data){
                if (m_status != Status::RUNNING) return;

                auto start_time = std::chrono::high_resolution_clock::now();
                try{
                    // Allocate market data memory
                    void* data_buffer = m_market_data_allocator.allocateQuoteBuffer();
                    if (!data_buffer){
                        return;
                    }

                    // Process market data
                    updateOrderBook(data.symbol);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                    updateMetrics(static_cast<double>(latency));
                }catch (...){

                }
            }

            bool tradingManager::start(){
                if (m_status != Status::STARTING && m_status != Status::PAUSED){
                    return false;
                }
                m_status = Status::RUNNING;
                return true;
            }

            bool tradingManager::stop() {
    if (m_status != Status::RUNNING && m_status != Status::PAUSED) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_transaction_mutex);
    m_status = Status::STOPPING;
    
    // Clean up any pending transactions
    {
        std::lock_guard<std::mutex> tx_lock(m_thread_transactions_mutex);
        for (auto& pair : m_thread_transactions) {
            m_transaction_allocator.rollbackTransaction(pair.second);
            m_transaction_allocator.endTransaction(pair.second);
        }
        m_thread_transactions.clear();
    }
    
    cleanupResources();
    m_status = Status::STARTING;
    return true;
}
            bool tradingManager::pause(){
                if (m_status != Status::RUNNING){
                    return false;
                }
                m_status = Status::PAUSED;
                return true;
            }

            bool tradingManager::resume(){
                if (m_status != Status::PAUSED){
                    return false;
                }
                m_status = Status::RUNNING;
                return true;
            }

            tradingManager::Status tradingManager::getStatus() const{
                return m_status;
            }

            tradingManager::Stats tradingManager::getStats() const{
                return Stats{
                    m_active_orders.load(),
                    m_pending_transactions.load(),
                    m_total_trades.load(),
                    calculateMemoryUsed(),
                    m_metrics -> avg_latency,
                    m_max_latency.load(),
                    calculateOrderRate(),   
                    calculateTradeRate()
                };
            }

            bool tradingManager::ishealthy() const{
                return m_status == Status::RUNNING && hasCapacity() && m_metrics -> avg_latency < 1000.0;
            }

            bool tradingManager::hasCapacity() const{
                return m_order_allocator.hasCapacity() && m_market_data_allocator.hasCapacity() && m_transaction_allocator.hasCapacity();
            }

            bool tradingManager::validateOrder(const order& ord) const{
                std::cerr << "validateOrder: OrderID=" << ord.order_id 
                            << ", Symbol=" << ord.symbol 
                            << ", Price=" << ord.price 
                            << ", Quantity=" << ord.quantity << "\n";

                return !ord.order_id.empty() && !ord.symbol.empty() && ord.price > 0.0 && ord.quantity > 0.0;
            }

            void tradingManager::updateMetrics(double latency){
                if (!m_metrics) return;
                double current_avg = m_metrics -> avg_latency;
                std::size_t count = m_metrics -> order_count + 1;
                m_metrics -> avg_latency = current_avg + (latency - current_avg) / count;

                double current_max = m_max_latency.load();
                while(latency > current_max){
                    m_max_latency.compare_exchange_weak(current_max,latency);
                }
                m_metrics -> order_count++;
                m_metrics -> last_update = std::chrono::high_resolution_clock::now();
            }

            void tradingManager::cleanupResources(){
                
                try {
        
                    m_active_orders.store(0);
                    m_total_trades.store(0);
                    m_pending_transactions.store(0);
                    m_total_latency.store(0.0);
                    m_max_latency.store(0.0);
                    
                    if (m_metrics) {
                        m_metrics->order_count = 0;
                        m_metrics->trade_count = 0;
                        m_metrics->avg_latency = 0.0;
                        m_metrics->last_update = std::chrono::high_resolution_clock::now();
                    }
                    
                    m_status = Status::STARTING;
                } catch (...) {
                    
                }
            }

            std::size_t tradingManager::calculateMemoryUsed() const{
                return m_order_allocator.getStats().total_memory_used + m_market_data_allocator.getStats().total_memory_used + 
                m_transaction_allocator.getStats().total_memory_used;
            }

            std::size_t tradingManager::calculateOrderRate() const{
                auto now = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_metrics->last_update).count();
                return duration > 0 ? m_metrics -> order_count / duration : 0;
            }

            std::size_t tradingManager::calculateTradeRate() const{
                auto now = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_metrics->last_update).count();
                return duration > 0 ? m_metrics -> trade_count / duration : 0;
            }
        }
    }
}
