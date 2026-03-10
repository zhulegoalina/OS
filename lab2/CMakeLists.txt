cmake_minimum_required(VERSION 3.10)
project(lab2_threads)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(thread_lib STATIC lib/thread_functions.cpp)

add_executable(ThreadLab src/main.cpp)
target_link_libraries(ThreadLab thread_lib)

enable_testing()
add_executable(ThreadTests tests/test_threads.cpp)
target_link_libraries(ThreadTests thread_lib)

if(WIN32)
    target_compile_definitions(ThreadLab PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(ThreadTests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

add_test(NAME ThreadTests COMMAND ThreadTests)