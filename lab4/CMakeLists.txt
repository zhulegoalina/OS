cmake_minimum_required(VERSION 3.15)
project(MessageQueue)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(receiver ${SRC_DIR}/receiver.cpp)
add_executable(sender ${SRC_DIR}/sender.cpp)

if(MSVC)
    target_compile_options(receiver PRIVATE /W4)
    target_compile_options(sender PRIVATE /W4)
    
    target_link_libraries(receiver)
    target_link_libraries(sender)
endif()

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Unit Tests
enable_testing()
add_executable(tests ${TESTS_DIR}/test_queue.cpp)
target_link_libraries(tests gtest_main)
target_include_directories(tests PRIVATE ${SRC_DIR})

include(GoogleTest)
gtest_discover_tests(tests)
