#include "../../../include/mercuryTrade/core/memory/mercMemoryTracker.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>

namespace mercuryTrade {
namespace core {
namespace memory {

MemoryTracker& MemoryTracker::instance() {
    static MemoryTracker instance;
    return instance;
}

void MemoryTracker::trackAllocation(void* ptr, std::size_t size, const char* file, int line) {
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    
    AllocationInfo info{
        size,
        std::chrono::steady_clock::now(),
        file,
        line,
        true
    };
    
    m_allocations[ptr] = info;
    
    // Update statistics
    m_totalAllocations++;
    m_activeAllocations++;
    m_totalBytesAllocated += size;
    m_currentBytesInUse += size;
    m_largestAllocation = std::max(m_largestAllocation.load(), size);
    m_peakBytesInUse = std::max(m_peakBytesInUse.load(), m_currentBytesInUse.load());
}

void MemoryTracker::trackDeallocation(void* ptr) {
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        if (it->second.isActive) {
            m_activeAllocations--;
            m_currentBytesInUse -= it->second.size;
            it->second.isActive = false;
        }
    }
}

MemoryTracker::MemoryStats MemoryTracker::getStats() const {
    return MemoryStats{
        m_totalAllocations,
        m_activeAllocations,
        m_totalBytesAllocated,
        m_currentBytesInUse,
        m_peakBytesInUse,
        m_largestAllocation
    };
}

std::vector<MemoryTracker::AllocationInfo> MemoryTracker::getActiveAllocations() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AllocationInfo> active;
    
    for (const auto& pair : m_allocations) {
        if (pair.second.isActive) {
            active.push_back(pair.second);
        }
    }
    
    return active;
}

void MemoryTracker::detectLeaks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    bool leaksFound = false;
    
    for (const auto& pair : m_allocations) {
        if (pair.second.isActive) {
            if (!leaksFound) {
                std::cout << "\nMemory leaks detected:\n";
                leaksFound = true;
            }
            
            std::cout << "Leak: " << pair.second.size << " bytes"
                     << " allocated at " << pair.second.file 
                     << ":" << pair.second.line << "\n";
        }
    }
    
    if (!leaksFound) {
        std::cout << "No memory leaks detected.\n";
    }
}

void MemoryTracker::printReport() const {
    auto stats = getStats();
    
    std::cout << "\nMemory Tracking Report:\n"
              << "=====================\n"
              << "Total allocations:     " << stats.totalAllocations << "\n"
              << "Active allocations:    " << stats.activeAllocations << "\n"
              << "Total bytes allocated: " << stats.totalBytesAllocated << "\n"
              << "Current bytes in use:  " << stats.currentBytesInUse << "\n"
              << "Peak bytes in use:     " << stats.peakBytesInUse << "\n"
              << "Largest allocation:    " << stats.largestAllocation << "\n\n";
              
    detectLeaks();
}

void MemoryTracker::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_allocations.clear();
    m_totalAllocations = 0;
    m_activeAllocations = 0;
    m_totalBytesAllocated = 0;
    m_currentBytesInUse = 0;
    m_peakBytesInUse = 0;
    m_largestAllocation = 0;
}

}}} // namespaces
