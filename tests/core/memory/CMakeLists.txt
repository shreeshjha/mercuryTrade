add_executable(mercAllocatorTest mercAllocatorTest.cpp)

target_link_libraries(mercAllocatorTest
    PRIVATE
        mercury_memory  # This should match the library name defined in src/CMakeLists.txt
)

# Add test to CTest
add_test(NAME mercAllocatorTest COMMAND mercAllocatorTest)
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
add_test(NAME mercAllocatorTest COMMAND mercAllocatorTest)
add_test(NAME mercAllocatorManagerTest COMMAND mercAllocatorManagerTest)