add_executable(mercAllocatorTest mercAllocatorTest.cpp)

target_link_libraries(mercAllocatorTest
    PRIVATE
        mercury_memory  # This should match the library name defined in src/CMakeLists.txt
)

# Add test to CTest
add_test(NAME mercAllocatorTest COMMAND mercAllocatorTest)
