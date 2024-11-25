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

# Options for sanitizers
option(ENABLE_ADDRESS_SANITIZER "Enable AddressSanitizer for memory debugging" OFF)
option(ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer for thread debugging" OFF)
option(ENABLE_UNDEFINED_SANITIZER "Enable UndefinedBehaviorSanitizer for debugging" OFF)

# Add sanitizer flags
if(ENABLE_ADDRESS_SANITIZER AND ENABLE_THREAD_SANITIZER)
    message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be enabled at the same time.")
endif()

if(ENABLE_ADDRESS_SANITIZER)
    message(STATUS "AddressSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

if(ENABLE_THREAD_SANITIZER)
    message(STATUS "ThreadSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O1")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
endif()

if(ENABLE_UNDEFINED_SANITIZER)
    message(STATUS "UndefinedBehaviorSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -O1")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
endif()

# Add pthread for Linux
if(UNIX AND NOT APPLE)
    add_compile_options(-pthread)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

# Add the include directory to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add the main library
add_subdirectory(src)

# Add verbose test output option
option(ENABLE_VERBOSE_TESTS "Enable verbose output in tests" ON)
if(ENABLE_VERBOSE_TESTS)
    add_definitions(-DVERBOSE_TESTS)
endif()

# Add tests
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

    # Apply sanitizers to all test targets
    if(ENABLE_ADDRESS_SANITIZER OR ENABLE_THREAD_SANITIZER OR ENABLE_UNDEFINED_SANITIZER)
        get_property(TEST_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests PROPERTY BUILDSYSTEM_TARGETS)
        foreach(TARGET ${TEST_TARGETS})
            if(ENABLE_ADDRESS_SANITIZER)
                set_target_properties(${TARGET} PROPERTIES
                    COMPILE_FLAGS "-fsanitize=address"
                    LINK_FLAGS "-fsanitize=address"
                )
            endif()
            if(ENABLE_THREAD_SANITIZER)
                set_target_properties(${TARGET} PROPERTIES
                    COMPILE_FLAGS "-fsanitize=thread"
                    LINK_FLAGS "-fsanitize=thread"
                )
            endif()
            if(ENABLE_UNDEFINED_SANITIZER)
                set_target_properties(${TARGET} PROPERTIES
                    COMPILE_FLAGS "-fsanitize=undefined"
                    LINK_FLAGS "-fsanitize=undefined"
                )
            endif()
        endforeach()
    endif()
endif()
