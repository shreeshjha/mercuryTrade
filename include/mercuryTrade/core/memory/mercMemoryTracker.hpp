#ifndef MERC_MEMORY_TRACKER_HPP
#define MERC_MEMORY_TRACKER_HPP

#include <cstddef>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>

namespace mercuryTrade {
namespace core {
namespace memory {

class MemoryTracker {
public:
    struct AllocationInfo {
        std::size_t size{0};
        std::chrono::steady_clock::time_point timestamp;
        const char* file{nullptr};  // Source file where allocation occurred
        int line{0};         // Line number where allocation occurred
        bool isActive{false};    // Whether this allocation is still active
    };

    void cleanup() noexcept{
        std::lock_guard<std::mutex> lock(m_mutex);
        m_allocations.clear();
    }

    struct MemoryStats {
        std::size_t totalAllocations;      // Total number of allocations
        std::size_t activeAllocations;     // Current number of active allocations
        std::size_t totalBytesAllocated;   // Total bytes allocated
        std::size_t currentBytesInUse;     // Current bytes in use
        std::size_t peakBytesInUse;        // Peak memory usage
        std::size_t largestAllocation;     // Size of largest single allocation
    };

    // Singleton access
    static MemoryTracker& instance();

    // Tracking methods
    void trackAllocation(void* ptr, std::size_t size, const char* file, int line);
    void trackDeallocation(void* ptr);

    // Statistics and reporting
    MemoryStats getStats() const;
    std::vector<AllocationInfo> getActiveAllocations() const;
    void detectLeaks() const noexcept;
    void printReport() const;

    // Reset tracking
    void reset();

private:
    MemoryTracker() = default;
    ~MemoryTracker() = default;
    
    // Prevent copying and assignment
    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker& operator=(const MemoryTracker&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<void*, AllocationInfo> m_allocations;
    
    // Statistics
    std::atomic<std::size_t> m_totalAllocations{0};
    std::atomic<std::size_t> m_activeAllocations{0};
    std::atomic<std::size_t> m_totalBytesAllocated{0};
    std::atomic<std::size_t> m_currentBytesInUse{0};
    std::atomic<std::size_t> m_peakBytesInUse{0};
    std::atomic<std::size_t> m_largestAllocation{0};
};

// Macro helpers for tracking
#ifdef MEMORY_TRACKING_ENABLED
    #define TRACK_ALLOCATION(ptr, size) \
        MemoryTracker::instance().trackAllocation(ptr, size, __FILE__, __LINE__)
    #define TRACK_DEALLOCATION(ptr) \
        MemoryTracker::instance().trackDeallocation(ptr)
#else
    #define TRACK_ALLOCATION(ptr, size) ((void)0)
    #define TRACK_DEALLOCATION(ptr) ((void)0)
#endif

}}} // namespaces

#endif // MERC_MEMORY_TRACKER_HPP
