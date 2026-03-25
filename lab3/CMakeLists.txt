cmake_minimum_required(VERSION 3.10)
project(lab3_sync)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(sync_lib STATIC lib/sync_functions.cpp)
add_executable(SyncLab src/main.cpp)
target_link_libraries(SyncLab sync_lib)

if(WIN32)
    target_compile_definitions(SyncLab PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

enable_testing()
add_executable(SyncTests tests/test_sync.cpp)
target_link_libraries(SyncTests sync_lib)
add_test(NAME SyncTests COMMAND SyncTests)