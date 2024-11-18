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
            , m_transaction_allocator(transactionAllocator::Config::getDefaultConfig())  // Explicit initialization
            , m_metrics(std::make_unique<performanceMetrics>())
              // Add this member variable
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
                if (m_metrics){
                    try{
                        if (m_status == Status::RUNNING){
                            stop();
                        }
                        m_metrics.reset();
                        cleanupResources();
                    }catch(...){
                    }
                }
            }

            bool tradingManager::beginTransaction(){
                if (m_status != Status::RUNNING){
                    return false;
                }
                try{
                    m_pending_transactions++;
                    return m_transaction_allocator.beginTransaction() != nullptr;
                }catch(...){
                    m_pending_transactions--;
                    return false;
                }
            }

            bool tradingManager::commitTransaction(){
                try{
                    if (m_pending_transactions > 0){
                        m_pending_transactions--;
                    }
                    return m_transaction_allocator.commitTransaction(nullptr);
                } catch (...){
                    return false;
                }
            }


            bool tradingManager::rollbackTransaction(){
                try{
                    if (m_pending_transactions > 0){
                        m_pending_transactions--;
                    }
                    return m_transaction_allocator.rollbackTransaction(nullptr);
                } catch(...){
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
                std::cout << "Starting submitOrder..." << std::endl;
                
                if (m_status != Status::RUNNING) {
                    std::cout << "Failed: Trading system not running. Status: " << static_cast<int>(m_status) << std::endl;
                    return false;
                }
                
                if (!validateOrder(ord)) {
                    std::cout << "Failed: Order validation failed" << std::endl;
                    return false;
                }
                
                auto start_time = std::chrono::high_resolution_clock::now();
                try {
                    std::cout << "Attempting to begin transaction..." << std::endl;
                    if (m_config.enable_transactions) {
                        if (!beginTransaction()) {
                            std::cout << "Failed: Could not begin transaction" << std::endl;
                            return false;
                        }
                        std::cout << "Transaction begun successfully" << std::endl;
                    }

                    std::cout << "Attempting to allocate order memory..." << std::endl;
                    OrderNode* order_node = m_order_allocator.allocateOrder();
                    if (!order_node) {
                        std::cout << "Failed: Could not allocate order memory" << std::endl;
                        if (m_config.enable_transactions) {
                            rollbackTransaction();
                        }
                        return false;
                    }
                    std::cout << "Order memory allocated successfully" << std::endl;

                    // Populate order node
                    order_node->price = ord.price;
                    order_node->quantity = ord.quantity;
                    m_order_allocator.registerOrder(ord.order_id, order_node);
                    std::cout << "Order node populated and registered" << std::endl;

                    try {
                        std::cout << "Updating order book for symbol: " << ord.symbol << std::endl;
                        updateOrderBook(ord.symbol);
                        std::cout << "Order book updated successfully" << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Exception in updateOrderBook: " << e.what() << std::endl;
                        m_order_allocator.deallocateOrder(order_node);
                        if (m_config.enable_transactions) {
                            rollbackTransaction();
                        }
                        return false;
                    }

                    // Update metrics
                    m_active_orders++;
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                    updateMetrics(static_cast<double>(latency));
                    std::cout << "Metrics updated" << std::endl;

                    if (m_config.enable_transactions) {
                        std::cout << "Committing transaction..." << std::endl;
                        if (!commitTransaction()) {
                            std::cout << "Failed: Could not commit transaction" << std::endl;
                            m_order_allocator.deallocateOrder(order_node);
                            return false;
                        }
                        std::cout << "Transaction committed successfully" << std::endl;
                    }
                    
                    std::cout << "Order submission completed successfully" << std::endl;
                    return true;
                    
                } catch (const std::exception& e) {
                    std::cout << "Exception in submitOrder: " << e.what() << std::endl;
                    if (m_config.enable_transactions) {
                        rollbackTransaction();
                    }
                    return false;
                } catch (...) {
                    std::cout << "Unknown exception in submitOrder" << std::endl;
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

            bool tradingManager::stop(){
                if (m_status != Status::RUNNING && m_status != Status::PAUSED){
                    return false;
                }
                m_status = Status::STOPPING;
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