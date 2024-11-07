# src/core/memory/CMakeLists.txt
add_library(mercury_memory
    mercAllocator.cpp
    mercAllocatorManager.cpp
    mercMemoryTracker.cpp
)

target_include_directories(mercury_memory
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Set compile flags for coverage if enabled
if(CODE_COVERAGE)
    target_compile_options(mercury_memory PRIVATE --coverage)
    target_link_options(mercury_memory PRIVATE --coverage)
endif()

# Set C++ standard for this target
set_target_properties(mercury_memory PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Link against pthread on Unix-like systems
if(UNIX)
    target_link_libraries(mercury_memory PUBLIC pthread)
endif()
