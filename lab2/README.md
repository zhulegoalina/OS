# Lab 2: Thread Creation in Windows

## Description
Multithreaded program that processes an integer array using two worker threads:
- **min_max**: finds minimum and maximum elements (sleeps 7ms after each comparison)
- **average**: calculates arithmetic mean (sleeps 12ms after each addition)

Main thread replaces min/max elements with average value.

## Files
- `include/thread_lab.h` - header with structures and declarations
- `lib/thread_functions.cpp` - thread implementations
- `src/main.cpp` - main program
- `tests/test_threads.cpp` - unit tests
- `CMakeLists.txt` - build configuration

## Build & Run
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run main program
Release\ThreadLab.exe

# Run tests
Release\ThreadTests.exe
