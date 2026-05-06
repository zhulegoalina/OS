cmake_minimum_required(VERSION 3.10)
project(lab5_named_pipe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(server src/server.cpp)
add_executable(client src/client.cpp)

if(WIN32)
    target_compile_definitions(server PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(client PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

enable_testing()
add_executable(named_pipe_tests tests/test_pipe.cpp)
target_compile_definitions(named_pipe_tests PRIVATE _CRT_SECURE_NO_WARNINGS)

add_test(NAME NamedPipeTests COMMAND named_pipe_tests)