#include "../../../include/mercuryTrade/core/memory/mercTransactionAllocator.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>
#include <random>

using namespace mercuryTrade::core::memory;

void verify(bool condition,const char* testName,const char* message){
    if (!condition){
        std::cerr<< testName << ": FAILED - " << message << std::endl;
        throw std::runtime_error(std::string(testName) + "failed: " + message);
    }
    std::cout<< testName << ": PASSED " << std::endl;
}

// Test basic transaction lifestyle
void testBasictTransactionLifestyle(){
    const char* TEST_NAME = "Basic Transaction Lifestyle Test";
    transactionAllocator allocator;

    // Begin Transaction
    transactionNode* transaction = allocator.beginTransaction();
    verify(transaction != nullptr , TEST_NAME , "Failed to Begin Transaction");
    verify(transaction -> status == transactionAllocator::transactionStatus::PENDING,TEST_NAME,"New transaction should be in pending state");
    
    // Register Transaction
    allocator.registerTransaction("TRANS1",transaction);
    
    // Verify Lookup
    transactionNode* found = allocator.findTransaction("TRANS1");
    verify(found == transaction , TEST_NAME , "Transaction Lookup Failed");

    // Committ Transaction
    bool commit_result = allocator.commitTransaction(transaction);
    verify(commit_result, TEST_NAME , "Transaction Commit Failed");
    verify(transaction -> status == transactionAllocator::transactionStatus::COMMITTED, TEST_NAME, "Transaction status shoudl be committed");

    // End transaction
    allocator.endTransaction(transaction);

    // Verify cleanup
    auto stats = allocator.getStats();
    verify(stats.active_transactions == 0, TEST_NAME , "Active transactions should be 0");
    verify(allocator.findTransaction("TRANS1") == nullptr , TEST_NAME, "Transaction should not be found after ending");
}

// Test transaction rollback
void testTransactionRollback(){
    const char* TEST_NAME = "Transaction rollback test";
    transactionAllocator allocator;

    // Begin transaction
    transactionNode* transaction = allocator.beginTransaction();
    verify(transaction != nullptr, TEST_NAME , "Failed to begin transaction");

    // Register Transaction
    allocator.registerTransaction("TRANS_ROLLBACK", transaction);

    // Rollback Transaction
    bool rollback_result = allocator.rollbackTransaction(transaction);
    verify(rollback_result , TEST_NAME , "Transaction rollback failed");
    verify(transaction -> status == transactionAllocator::transactionStatus::ROLLED_BACK, TEST_NAME , "Transaction status should be rolled back");

    // End transaction
    allocator.endTransaction(transaction);

    // Verify stats
    auto stats = allocator.getStats();
    verify(stats.rollbacks_performed == 1 , TEST_NAME , "Rolled backs count should be 1");
    verify(stats.active_transactions == 0, TEST_NAME , "Active transactions should be 0");
}

// Test batch management
void testBatchManagement(){
    const char* TEST_NAME = "Batch Management Test";

    // Create allocator with small batch size for testing
    transactionAllocator::Config config {
        100, // Max transactions
        5, // Max batches
        10, // Batch Size
        128, // transaction_data_size
        true // enable_rollback
    };

    transactionAllocator allocator(config);
    std::vector<transactionNode*> transactions;

    // Fill up a batch
    for (int i = 0 ; i < config.batch_size ; i++){
        transactionNode* trans = allocator.beginTransaction();
        verify(trans != nullptr ,  TEST_NAME , "Failed to allocate transaction");
        transactions.push_back(trans);
    }

    // Verify batch utilization
    auto stats = allocator.getStats();
    verify(stats.active_batches == 1 , TEST_NAME , "Should have one active batch");
    verify(stats.batch_allocations == 1 , TEST_NAME , "Should have one batch allocation");

    // Cleanup
    for (auto* trans : transactions){
        allocator.endTransaction(trans);
    }
    // Verify cleanup
    stats = allocator.getStats();
    verify(stats.active_transactions == 0 , TEST_NAME , "Active transactions should be 0");
    verify(stats.active_batches == 0 , TEST_NAME , "Active batches should be 0");
}

// Test Concurrent Transaction
void testConcurrentTransaction(){
    const char* TEST_NAME = "Concurrent Transactions Test";

    transactionAllocator allocator;
    std::atomic<bool> test_failed{false};
    std::atomic<int> total_transactions{0};

    auto thread_func = [&](int thread_id){
        try{
            std::vector<transactionNode*> thread_transactions;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1,100);
            for (int i = 0 ; i < 50 ; i++){
                // Begin new transaction
                transactionNode* trans = allocator.beginTransaction();
                if (trans) {
                    // Register transaction
                    std::string trans_id = "TRANS_" + std::to_string(thread_id) + "_" + std::to_string(i);
                    allocator.registerTransaction(trans_id , trans);
                    thread_transactions.push_back(trans);
                    total_transactions++;

                    // Randomly committ or rollback
                    if (dis(gen) > 50){
                        allocator.commitTransaction(trans);
                    } else{
                        allocator.rollbackTransaction(trans);
                    }
                }
                // Randomly end some transactions
                if (!thread_transactions.empty() && dis(gen) > 70){
                    size_t index = dis(gen) % thread_transactions.size();
                    allocator.endTransaction(thread_transactions[index]);
                    thread_transactions[index] = thread_transactions.back();
                    thread_transactions.pop_back();
                    total_transactions--;
                }
            }
            // For cleaning up remaining transactions
            for (auto* trans : thread_transactions){
                allocator.endTransaction(trans);
                total_transactions--;
            }
        }catch(...){
            test_failed = true;
        }
    };

    // Create and run threads
    std::vector<std::thread> threads;
    for (int i = 0 ; i < 4 ; i++){
        threads.emplace_back(thread_func , i);
    }

    // Wait for threads to complete
    for (auto& thread : threads){
        thread.join();
    }

    // Verify result
    verify(!test_failed , TEST_NAME , "Concurrent operations test failed");
    verify(total_transactions == 0, TEST_NAME , "Not all transactions were properly ended");

    auto stats = allocator.getStats();
    verify(stats.active_transactions == 0 ,TEST_NAME , "Active transactions should be 0");
    verify(stats.active_batches == 0, TEST_NAME , "Active batches should be 0");
}

// Test capacity limits
void testCapacityLimits(){
    const char* TEST_NAME = "Capacity Limits Test";

    // Create allocator with small limits
    transactionAllocator::Config config{
        5, // max_transactions
        2, // max_batches
        3, // batch_size
        128, // transaction_data_size
        true // enable_rollback
    };

    transactionAllocator allocator(config);
    std::vector<transactionNode*> transactions;

    // Try to exceed transaction limit
    for (int i = 0 ; i < 7 ; i++){
        transactionNode* trans = allocator.beginTransaction();
        if (trans){
            transactions.push_back(trans);
        }
    }
    verify(transactions.size() == 5, TEST_NAME , "Should not exceed max transactions");
    // Clean up
    for (auto* trans : transactions){
        allocator.endTransaction(trans);
    }
    auto stats = allocator.getStats();
    verify(stats.active_transactions == 0 , TEST_NAME , "Active transactions should be 0");
    verify(stats.active_batches == 0 , TEST_NAME , "Active baches should be 0");
}

int main(){
    std::cout << "\n Starting Transaction Allocator Test...\n" << std::endl;
    try{
        testBasictTransactionLifestyle();
        testTransactionRollback();
        testBatchManagement();
        testConcurrentTransaction();
        testCapacityLimits();
        std::cout << "\n All transaction allocator tests completed sucessfully! \n" <<std::endl;
        return 0;
    } catch (const std::exception &e){
        std::cerr << "\n Test Failed: " << e.what() << std::endl;
        return 1;
    }
}