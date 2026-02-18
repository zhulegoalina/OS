cmake_minimum_required(VERSION 3.10)
project(lab1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(Creator src/Creator.cpp)
add_executable(Reporter src/Reporter.cpp)
add_executable(Main src/Main.cpp)

if(WIN32)
    target_compile_definitions(Creator PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(Reporter PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(Main PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

enable_testing()
add_executable(ProcessTests tests/test_processes.cpp)
add_test(NAME ProcessTests COMMAND ProcessTests)