cmake_minimum_required(VERSION 3.14)
project(mercuryTrade VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#Add AddressSanitizer for memory debugging
option(ENABLE_SANITIZERS "Enable AddressSanitizer for debugging" ON)
if(ENABLE_SANITIZERS AND NOT MSVC)
    message(STATUS "AddressSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

# Add the include directory to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add the main library
add_subdirectory(src)

# Add tests
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
