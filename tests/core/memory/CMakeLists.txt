# Remove previous test entries if they exist
if(TARGET mercAllocatorTest)
    remove_test(mercAllocatorTest)
endif()

if(TARGET mercAllocatorManagerTest)
    remove_test(mercAllocatorManagerTest)
endif()

# Add test executables
add_executable(mercAllocatorTest mercAllocatorTest.cpp)
add_executable(mercAllocatorManagerTest mercAllocatorManagerTest.cpp)
add_executable(mercMemoryTrackingTest mercMemoryTrackingTest.cpp)
add_executable(mercMarketDataAllocatorTest mercMarketDataAllocatorTest.cpp)  
add_executable(mercOrderBookAllocatorTest mercOrderBookAllocatorTest.cpp)
add_executable(mercTransactionAllocatorTest mercTransactionAllocatorTest.cpp)
add_executable(mercTradingManagerTest mercTradingManagerTest.cpp)

# Link against the library
target_link_libraries(mercAllocatorTest
    PRIVATE
        mercury_memory
)

target_link_libraries(mercAllocatorManagerTest
    PRIVATE
        mercury_memory
)

target_link_libraries(mercMemoryTrackingTest 
    PRIVATE 
        mercury_memory
)

target_link_libraries(mercMarketDataAllocatorTest 
    PRIVATE 
        mercury_memory
)  

target_link_libraries(mercOrderBookAllocatorTest 
    PRIVATE 
        mercury_memory
)

target_link_libraries(mercTransactionAllocatorTest 
    PRIVATE 
        mercury_memory
)

target_link_libraries(mercTradingManagerTest 
    PRIVATE 
        mercury_memory
)

# Add tests to CTest
add_test(NAME AllocatorTest COMMAND mercAllocatorTest)          # Changed test name
add_test(NAME AllocatorManagerTest COMMAND mercAllocatorManagerTest)  # Different test name
add_test(NAME MemoryTrackingTest COMMAND mercMemoryTrackingTest)
add_test(NAME MarketDataAllocatorTest COMMAND mercMarketDataAllocatorTest)  
add_test(NAME OrderBookAllocatorTest COMMAND mercOrderBookAllocatorTest)
add_test(NAME TransactionAllocatorTest COMMAND mercTransactionAllocatorTest)
add_test(NAME TradingManagerTest COMMAND mercTradingManagerTest)
