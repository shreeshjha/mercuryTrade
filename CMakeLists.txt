cmake_minimum_required(VERSION 3.14)
project(mercuryTrade VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug settings
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# macOS specific settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

# Global include directories
# include_directories(
#     ${PROJECT_SOURCE_DIR}/include
# )
find_package(nlohmann_json REQUIRED)
# include_directories(${PROJECT_SOURCE_DIR}/json/include)


# Add JSON library (using the one in your project)
# add_library(nlohmann_json INTERFACE)
# target_include_directories(nlohmann_json INTERFACE ${PROJECT_SOURCE_DIR}/json)


# Add the main library
add_subdirectory(src)

# Add API library
add_library(mercury_api
    src/api/auth/AuthController.cpp
    src/api/market/MarketDataController.cpp
    src/api/orders/OrderController.cpp
    src/services/OrderService.cpp
    src/services/UserService.cpp          
    src/services/MarketDataService.cpp    
    src/services/OrderBookService.cpp     
    src/http/Server.cpp
)

target_include_directories(mercury_api PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_library(mercury_http
    src/http/Server.cpp
    # Add other necessary .cpp files
)

target_include_directories(mercury_http PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)


target_link_libraries(mercury_api
    PRIVATE
        mercury_memory
        nlohmann_json
)
target_link_libraries(mercury_http PUBLIC
    nlohmann_json
)
# Add server executable
add_executable(mercury_server src/api/main.cpp)
target_link_libraries(mercury_server
    PRIVATE
        mercury_api
        mercury_memory
        mercury_http
)

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

# Add tests
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
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

# Apply sanitizers to API library and server
if(ENABLE_ADDRESS_SANITIZER)
    target_compile_options(mercury_api PRIVATE -fsanitize=address)
    target_link_options(mercury_api PRIVATE -fsanitize=address)
    target_compile_options(mercury_server PRIVATE -fsanitize=address)
    target_link_options(mercury_server PRIVATE -fsanitize=address)
endif()

if(ENABLE_THREAD_SANITIZER)
    target_compile_options(mercury_api PRIVATE -fsanitize=thread)
    target_link_options(mercury_api PRIVATE -fsanitize=thread)
    target_compile_options(mercury_server PRIVATE -fsanitize=thread)
    target_link_options(mercury_server PRIVATE -fsanitize=thread)
endif()

if(ENABLE_UNDEFINED_SANITIZER)
    target_compile_options(mercury_api PRIVATE -fsanitize=undefined)
    target_link_options(mercury_api PRIVATE -fsanitize=undefined)
    target_compile_options(mercury_server PRIVATE -fsanitize=undefined)
    target_link_options(mercury_server PRIVATE -fsanitize=undefined)
endif()





# cmake_minimum_required(VERSION 3.14)
# project(mercuryTrade VERSION 1.0.0 LANGUAGES CXX)

# # Set C++ standard
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Debug settings
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# # Add compiler warnings
# if(MSVC)
#     add_compile_options(/W4)
# else()
#     add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# # Options for sanitizers
# option(ENABLE_ADDRESS_SANITIZER "Enable AddressSanitizer for memory debugging" OFF)
# option(ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer for thread debugging" OFF)
# option(ENABLE_UNDEFINED_SANITIZER "Enable UndefinedBehaviorSanitizer for debugging" OFF)

# # Add sanitizer flags
# if(ENABLE_ADDRESS_SANITIZER AND ENABLE_THREAD_SANITIZER)
#     message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be enabled at the same time.")
# endif()

# if(ENABLE_ADDRESS_SANITIZER)
#     message(STATUS "AddressSanitizer enabled")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")
#     set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
# endif()

# if(ENABLE_THREAD_SANITIZER)
#     message(STATUS "ThreadSanitizer enabled")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O1")
#     set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
# endif()

# if(ENABLE_UNDEFINED_SANITIZER)
#     message(STATUS "UndefinedBehaviorSanitizer enabled")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -O1")
#     set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
# endif()

# # Add pthread for Linux
# if(UNIX AND NOT APPLE)
#     add_compile_options(-pthread)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
# endif()

# # Configure OpenSSL
# if(APPLE)
#     set(SSL_LIB "/opt/homebrew/opt/openssl@1.1/lib/libssl.dylib")
#     set(CRYPTO_LIB "/opt/homebrew/opt/openssl@1.1/lib/libcrypto.dylib")
#     set(OPENSSL_INCLUDE_DIR "/opt/homebrew/opt/openssl@1.1/include")
# endif()


# _package(Boost REQUIRED)

# # Add ASIO as header-only library
# add_library(asio INTERFACE)
# target_include_directories(asio INTERFACE 
#     ${CMAKE_SOURCE_DIR}/asio/asio/include
# )
# target_compile_definitions(asio INTERFACE 
#     ASIO_STANDALONE
#     ASIO_HAS_STD_ARRAY
#     ASIO_HAS_STD_SHARED_ARRAY
#     ASIO_HAS_CSTDINT
#     ASIO_HAS_STD_ADDRESSOF
#     ASIO_HAS_STD_TYPE_TRAITS
#     CROW_ENABLE_SSL
#     ASIO_NO_DEPRECATED 
# )

# # Add Crow as header-only library
# add_library(Crow INTERFACE)
# target_include_directories(Crow INTERFACE 
#     ${CMAKE_SOURCE_DIR}/Crow/include
#     ${CMAKE_SOURCE_DIR}/include
#     ${OPENSSL_INCLUDE_DIR}
# )
# target_link_libraries(Crow INTERFACE 
#     asio
#     ${SSL_LIB}
#     ${CRYPTO_LIB}
# )

# # Add JSON library
# add_library(nlohmann_json INTERFACE)
# target_include_directories(nlohmann_json INTERFACE ${CMAKE_SOURCE_DIR}/json/include)

# # Add the include directory to the include path
# include_directories(
#     ${PROJECT_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/asio/asio/include
#     ${OPENSSL_INCLUDE_DIR}
#     ${Boost_INCLUDE_DIRS}
# )

# # Add the main library
# add_subdirectory(src)

# # Add API library
# add_library(mercury_api
#     src/api/auth/AuthController.cpp
#     src/api/market/MarketDataController.cpp
#     src/api/orders/OrderController.cpp
#     src/services/OrderService.cpp
# )

# target_include_directories(mercury_api PUBLIC
#     ${PROJECT_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/Crow/include
#     ${CMAKE_SOURCE_DIR}/asio/asio/include
#     ${OPENSSL_INCLUDE_DIR}
#     ${Boost_INCLUDE_DIRS}
# )

# target_compile_definitions(mercury_api PRIVATE ASIO_STANDALONE)

# target_link_libraries(mercury_api
#     PRIVATE
#         mercury_memory
#         Crow
#         nlohmann_json
#         ${SSL_LIB}
#         ${CRYPTO_LIB}
#         ${Boost_LIBRARIES}
# )

# # Add server executable
# add_executable(mercury_server src/api/main.cpp)
# target_link_libraries(mercury_server
#     PRIVATE
#         mercury_api
# )

# target_include_directories(mercury_server PRIVATE
#     ${PROJECT_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/Crow/include
#     ${CMAKE_SOURCE_DIR}/asio/asio/include
#     ${OPENSSL_INCLUDE_DIR}
#     ${Boost_INCLUDE_DIRS}
# )

# target_compile_definitions(mercury_server PRIVATE ASIO_STANDALONE)

# # Add verbose test output option
# option(ENABLE_VERBOSE_TESTS "Enable verbose output in tests" ON)
# if(ENABLE_VERBOSE_TESTS)
#     add_definitions(-DVERBOSE_TESTS)
# endif()

# # Add tests
# option(BUILD_TESTS "Build the tests" ON)
# if(BUILD_TESTS)
#     enable_testing()
#     add_subdirectory(tests)
    
#     # Apply sanitizers to all test targets
#     if(ENABLE_ADDRESS_SANITIZER OR ENABLE_THREAD_SANITIZER OR ENABLE_UNDEFINED_SANITIZER)
#         get_property(TEST_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests PROPERTY BUILDSYSTEM_TARGETS)
#         foreach(TARGET ${TEST_TARGETS})
#             if(ENABLE_ADDRESS_SANITIZER)
#                 set_target_properties(${TARGET} PROPERTIES
#                     COMPILE_FLAGS "-fsanitize=address"
#                     LINK_FLAGS "-fsanitize=address"
#                 )
#             endif()
#             if(ENABLE_THREAD_SANITIZER)
#                 set_target_properties(${TARGET} PROPERTIES
#                     COMPILE_FLAGS "-fsanitize=thread"
#                     LINK_FLAGS "-fsanitize=thread"
#                 )
#             endif()
#             if(ENABLE_UNDEFINED_SANITIZER)
#                 set_target_properties(${TARGET} PROPERTIES
#                     COMPILE_FLAGS "-fsanitize=undefined"
#                     LINK_FLAGS "-fsanitize=undefined"
#                 )
#             endif()
#         endforeach()
#     endif()
# endif()

# # Apply sanitizers to API library and server
# if(ENABLE_ADDRESS_SANITIZER)
#     target_compile_options(mercury_api PRIVATE -fsanitize=address)
#     target_link_options(mercury_api PRIVATE -fsanitize=address)
#     target_compile_options(mercury_server PRIVATE -fsanitize=address)
#     target_link_options(mercury_server PRIVATE -fsanitize=address)
# endif()

# if(ENABLE_THREAD_SANITIZER)
#     target_compile_options(mercury_api PRIVATE -fsanitize=thread)
#     target_link_options(mercury_api PRIVATE -fsanitize=thread)
#     target_compile_options(mercury_server PRIVATE -fsanitize=thread)
#     target_link_options(mercury_server PRIVATE -fsanitize=thread)
# endif()

# if(ENABLE_UNDEFINED_SANITIZER)
#     target_compile_options(mercury_api PRIVATE -fsanitize=undefined)
#     target_link_options(mercury_api PRIVATE -fsanitize=undefined)
#     target_compile_options(mercury_server PRIVATE -fsanitize=undefined)
#     target_link_options(mercury_server PRIVATE -fsanitize=undefined)
# endif()

# # Additional macOS-specific settings
# if(APPLE)
#     set(CMAKE_MACOSX_RPATH 1)
# endif()



# cmake_minimum_required(VERSION 3.14)
# project(mercuryTrade VERSION 1.0.0 LANGUAGES CXX)

# # Set C++ standard
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Debug settings
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# # Add compiler warnings
# if(MSVC)
#     add_compile_options(/W4)
# else()
#     add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# # Options for sanitizers
# option(ENABLE_ADDRESS_SANITIZER "Enable AddressSanitizer for memory debugging" OFF)
# option(ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer for thread debugging" OFF)
# option(ENABLE_UNDEFINED_SANITIZER "Enable UndefinedBehaviorSanitizer for debugging" OFF)

# # Add sanitizer flags
# if(ENABLE_ADDRESS_SANITIZER AND ENABLE_THREAD_SANITIZER)
#     message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be enabled at the same time.")
# endif()

# if(ENABLE_ADDRESS_SANITIZER)
#     message(STATUS "AddressSanitizer enabled")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")
#     set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
# endif()

# if(ENABLE_THREAD_SANITIZER)
#     message(STATUS "ThreadSanitizer enabled")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O1")
#     set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
# endif()

# if(ENABLE_UNDEFINED_SANITIZER)
#     message(STATUS "UndefinedBehaviorSanitizer enabled")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -O1")
#     set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
# endif()

# # Add pthread for Linux
# if(UNIX AND NOT APPLE)
#     add_compile_options(-pthread)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
# endif()

# # Add Crow as a header-only library
# add_library(Crow INTERFACE)
# target_include_directories(Crow INTERFACE ${CMAKE_SOURCE_DIR}/Crow/include)

# # Add jwt-cpp as a header-only library
# add_library(jwt-cpp INTERFACE)
# target_include_directories(jwt-cpp INTERFACE ${CMAKE_SOURCE_DIR}/jwt-cpp/include)

# # Add nlohmann_json as a header-only library
# add_library(nlohmann_json INTERFACE)
# target_include_directories(nlohmann_json INTERFACE ${CMAKE_SOURCE_DIR}/json/include)




# # Add the include directory to the include path
# include_directories(${PROJECT_SOURCE_DIR}/include)

# # Add the main library
# add_subdirectory(src)

# # Add API library
# add_library(mercury_api
#     src/api/auth/AuthController.cpp
#     src/api/market/MarketDataController.cpp
#     src/api/routes/AuthRoutes.cpp
#     src/api/routes/MarketDataRoutes.cpp
# )

# target_link_libraries(mercury_api
#     PRIVATE
#         mercury_memory
#         Crow
#         jwt-cpp
#         nlohmann_json
# )

# # Add server executable
# add_executable(mercury_server src/api/main.cpp)
# target_link_libraries(mercury_server
#     PRIVATE
#         mercury_api
# )

# target_include_directories(mercury_api PRIVATE ${CMAKE_SOURCE_DIR}/asio/asio/include)
# target_include_directories(mercury_server PRIVATE ${CMAKE_SOURCE_DIR}/asio/asio/include)

# # Add verbose test output option
# option(ENABLE_VERBOSE_TESTS "Enable verbose output in tests" ON)
# if(ENABLE_VERBOSE_TESTS)
#     add_definitions(-DVERBOSE_TESTS)
# endif()

# # Add tests
# option(BUILD_TESTS "Build the tests" ON)
# if(BUILD_TESTS)
#     enable_testing()
#     add_subdirectory(tests)
    
#     # Apply sanitizers to all test targets
#     if(ENABLE_ADDRESS_SANITIZER OR ENABLE_THREAD_SANITIZER OR ENABLE_UNDEFINED_SANITIZER)
#         get_property(TEST_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests PROPERTY BUILDSYSTEM_TARGETS)
#         foreach(TARGET ${TEST_TARGETS})
#             if(ENABLE_ADDRESS_SANITIZER)
#                 set_target_properties(${TARGET} PROPERTIES
#                     COMPILE_FLAGS "-fsanitize=address"
#                     LINK_FLAGS "-fsanitize=address"
#                 )
#             endif()
#             if(ENABLE_THREAD_SANITIZER)
#                 set_target_properties(${TARGET} PROPERTIES
#                     COMPILE_FLAGS "-fsanitize=thread"
#                     LINK_FLAGS "-fsanitize=thread"
#                 )
#             endif()
#             if(ENABLE_UNDEFINED_SANITIZER)
#                 set_target_properties(${TARGET} PROPERTIES
#                     COMPILE_FLAGS "-fsanitize=undefined"
#                     LINK_FLAGS "-fsanitize=undefined"
#                 )
#             endif()
#         endforeach()
#     endif()
# endif()

# # Apply sanitizers to API library and server
# if(ENABLE_ADDRESS_SANITIZER)
#     target_compile_options(mercury_api PRIVATE -fsanitize=address)
#     target_link_options(mercury_api PRIVATE -fsanitize=address)
#     target_compile_options(mercury_server PRIVATE -fsanitize=address)
#     target_link_options(mercury_server PRIVATE -fsanitize=address)
# endif()

# if(ENABLE_THREAD_SANITIZER)
#     target_compile_options(mercury_api PRIVATE -fsanitize=thread)
#     target_link_options(mercury_api PRIVATE -fsanitize=thread)
#     target_compile_options(mercury_server PRIVATE -fsanitize=thread)
#     target_link_options(mercury_server PRIVATE -fsanitize=thread)
# endif()

# if(ENABLE_UNDEFINED_SANITIZER)
#     target_compile_options(mercury_api PRIVATE -fsanitize=undefined)
#     target_link_options(mercury_api PRIVATE -fsanitize=undefined)
#     target_compile_options(mercury_server PRIVATE -fsanitize=undefined)
#     target_link_options(mercury_server PRIVATE -fsanitize=undefined)
# endif()
