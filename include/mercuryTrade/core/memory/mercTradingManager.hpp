#ifndef MERC_TRADING_MANAGER_HPP
#define MERC_TRADING_MANAGER_HPP

#include "mercOrderBookAllocator.hpp"
#include "mercTransactionAllocator.hpp"
#include "mercMarketDataAllocator.hpp"
#include <string>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <vector>

namespace mercuryTrade{
    namespace core{
        namespace memory{
            struct order;
            struct marketData;
            struct trade;

            class tradingManager{
                public:
                    struct Config{
                        // Configuration for the trading system
                        std::size_t max_orders; // max number of orders
                        std::size_t max_symbols; // max number of symbols
                        std::size_t market_data_size;  // size of market data buffer
                        bool enable_transactions; // whether to enable transaction tracking

                        static Config getDefautltConfig(){
                            return Config{
                                1000000,  // max orders
                                10000,  // max symbols
                                1024,  // market_data_size
                                true  //enable_transasctions
                            };
                        }
                    };
                    // Statistics about the trading system
                    struct Stats{
                        std::size_t active_orders;
                        std::size_t pending_transactions;
                        std::size_t total_trades;
                        std::size_t memory_used;
                        double avg_latency;
                        double max_latency;
                        std::size_t order_rate;
                        std::size_t trade_rate;
                    };

                    // Trading system status
                    enum class Status{
                        STARTING,
                        RUNNING,
                        PAUSED,
                        STOPPING,
                        ERROR
                    };

                    // Constructor and Destructor
                    explicit tradingManager(const Config& config = Config::getDefautltConfig());
                    ~tradingManager() noexcept;

                    // Prevent copying
                    tradingManager(const tradingManager&) = delete;
                    tradingManager& operator = (const tradingManager&) = delete;

                    // Allow moving
                    tradingManager(tradingManager&&) noexcept = default;
                    tradingManager& operator = (tradingManager&&) noexcept = default;

                    // Core trading methods
                    bool submitOrder(const order& ord);
                    bool cancelOrder(const std::string& order_id);
                    bool modifyOrder(const std::string& order_id, const order& new_order);

                    // Market Data Handling
                    void handleMarketData(const marketData& data);
                    void updateOrderBook(const std::string& symbol);

                    // Transaction management
                    bool beginTransaction();
                    bool commitTransaction();
                    bool rollbackTransaction();

                    // System management
                    bool start();
                    bool stop();
                    bool pause();
                    bool resume();

                    // Status and monitoring
                    Status getStatus() const;
                    Stats getStats() const;
                    bool ishealthy() const;

                    // Memory management
                    void optimizeMemory();
                    bool hasCapacity() const;

                private:
                    Config m_config;
                    Status m_status{Status::STARTING};

                    // Memory allocators
                    OrderBookAllocator m_order_allocator{};
                    marketDataAllocator m_market_data_allocator{};
                    transactionAllocator m_transaction_allocator{};

                    // System Statistics
                    std::atomic<std::size_t> m_active_orders{0};
                    std::atomic<std::size_t> m_total_trades{0};
                    std::atomic<std::size_t> m_pending_transactions{0};
                    std::atomic<double> m_total_latency{0.0};
                    std::atomic<double> m_max_latency{0.0};

                    // Performance Monitoring
                    struct performanceMetrics{
                        std::chrono::high_resolution_clock::time_point last_update{};
                        std::size_t order_count{0};
                        std::size_t trade_count{0};
                        double avg_latency{0.0};
                    };
                    std::unique_ptr<performanceMetrics> m_metrics;

                    // Internal methods
                    bool validateOrder(const order& ord) const;
                    void updateMetrics(double latency);
                    void cleanupResources();

                    // Helper methods for stats calculation
                    std::size_t calculateMemoryUsed() const;
                    std::size_t calculateOrderRate() const;
                    std::size_t calculateTradeRate() const;
            };

            struct order{
                std::string order_id;
                std::string symbol;
                double price;
                double quantity;
                bool is_buy;
                std::chrono::system_clock::time_point timestamp;
            };

            struct marketData{
                std::string symbol;
                double bid;
                double ask;
                double last;
                double volume;
                std::chrono::system_clock::time_point timestamp;
            };

            struct trade{
                std::string trade_id;
                std::string buy_order_id;
                std::string sell_order_id;
                std::string symbol;
                double price;
                double quantity;
                std::chrono::system_clock::time_point timestamp;
            };
        }
    }
}






#endif