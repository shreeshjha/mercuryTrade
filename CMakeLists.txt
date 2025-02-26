cmake_minimum_required(VERSION 3.14)
project(mercuryTrade VERSION 1.0.0 LANGUAGES CXX)

# Basic settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Platform-specific settings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    # Add Homebrew paths for macOS
    set(PQXX_INCLUDE_PATH "/opt/homebrew/include")
    set(PQXX_LIBRARY_PATH "/opt/homebrew/lib/libpqxx.dylib")
    set(SPDLOG_INCLUDE_PATH "/opt/homebrew/include")
    set(SPDLOG_LIBRARY_PATH "/opt/homebrew/lib/libspdlog.dylib")
endif()

if(UNIX AND NOT APPLE)
    add_compile_options(-pthread)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
    set(PQXX_INCLUDE_PATH "/usr/include")
    set(PQXX_LIBRARY_PATH "/usr/lib/libpqxx.so")
    set(SPDLOG_INCLUDE_PATH "/usr/include")
    set(SPDLOG_LIBRARY_PATH "/usr/lib/libspdlog.so")
endif()

# Manually set CMAKE_PREFIX_PATH to help CMake find packages
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/opt/homebrew;/opt/homebrew/lib/cmake/pqxx")

# Required packages
find_package(nlohmann_json REQUIRED)

# Manually configure pqxx since pqxxConfig.cmake is missing
include_directories(${PQXX_INCLUDE_PATH})
link_directories(${PQXX_LIBRARY_PATH})

# WebSocket++ as a header-only library
add_library(websocketpp INTERFACE)
target_include_directories(websocketpp INTERFACE 
    ${PROJECT_SOURCE_DIR}/external/websocketpp
)

# Manually create pqxx interface library
add_library(pqxx INTERFACE)
target_include_directories(pqxx INTERFACE ${PQXX_INCLUDE_PATH})
target_link_libraries(pqxx INTERFACE ${PQXX_LIBRARY_PATH})

# Manually create spdlog interface library
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${SPDLOG_INCLUDE_PATH})
target_link_libraries(spdlog INTERFACE ${SPDLOG_LIBRARY_PATH})

# Global include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add main source directory
add_subdirectory(src)

# Libraries
add_library(mercury_api
    src/api/auth/AuthController.cpp
    src/api/market/MarketDataController.cpp
    src/api/orders/OrderController.cpp
    src/services/OrderService.cpp
    src/services/UserService.cpp          
    src/services/MarketDataService.cpp    
    src/services/OrderBookService.cpp     
)

add_library(mercury_http
    src/http/Server.cpp
)

# Configure library includes and links
target_include_directories(mercury_api PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PQXX_INCLUDE_PATH}
    ${SPDLOG_INCLUDE_PATH}
)

target_include_directories(mercury_http PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(mercury_api
    PRIVATE
        mercury_memory
        nlohmann_json::nlohmann_json
        pqxx
        spdlog
        websocketpp
)

target_link_libraries(mercury_http PUBLIC
    nlohmann_json::nlohmann_json
)

# Server executable
add_executable(mercury_server src/api/main.cpp)
target_link_libraries(mercury_server
    PRIVATE
        mercury_api
        mercury_memory
        mercury_http
)

# Sanitizer options
option(ENABLE_ADDRESS_SANITIZER "Enable AddressSanitizer" OFF)
option(ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer" OFF)
option(ENABLE_UNDEFINED_SANITIZER "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ADDRESS_SANITIZER AND ENABLE_THREAD_SANITIZER)
    message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be enabled simultaneously")
endif()

# Function to apply sanitizer settings
function(apply_sanitizer target sanitizer)
    if(${sanitizer})
        target_compile_options(${target} PRIVATE -fsanitize=${sanitizer} -g -O1)
        target_link_options(${target} PRIVATE -fsanitize=${sanitizer})
    endif()
endfunction()

# Apply sanitizers to targets
foreach(TARGET mercury_api mercury_server)
    apply_sanitizer(${TARGET} "address")
    apply_sanitizer(${TARGET} "thread")
    apply_sanitizer(${TARGET} "undefined")
endforeach()

# Tests configuration
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    if(ENABLE_ADDRESS_SANITIZER OR ENABLE_THREAD_SANITIZER OR ENABLE_UNDEFINED_SANITIZER)
        get_property(TEST_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests PROPERTY BUILDSYSTEM_TARGETS)
        foreach(TARGET ${TEST_TARGETS})
            apply_sanitizer(${TARGET} "address")
            apply_sanitizer(${TARGET} "thread")
            apply_sanitizer(${TARGET} "undefined")
        endforeach()
    endif()
endif()
