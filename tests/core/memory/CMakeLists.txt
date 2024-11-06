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

# Link against the library
target_link_libraries(mercAllocatorTest
    PRIVATE
        mercury_memory
)

target_link_libraries(mercAllocatorManagerTest
    PRIVATE
        mercury_memory
)

# Add tests to CTest
add_test(NAME AllocatorTest COMMAND mercAllocatorTest)          # Changed test name
add_test(NAME AllocatorManagerTest COMMAND mercAllocatorManagerTest)  # Different test name
