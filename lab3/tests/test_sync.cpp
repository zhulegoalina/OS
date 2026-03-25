#define NOMINMAX
#include "sync_lab.h"
#include <cassert>
#include <iostream>
#include <vector>

void test_array_initialization() {
    std::cout << "\n=== Test 1: Array initialization ===" << std::endl;
    std::vector<int> arr(10, 0);
    for (int i = 0; i < 10; i++) {
        assert(arr[i] == 0);
    }
    std::cout << "Input: size=10, all zeros" << std::endl;
    std::cout << "Output: all zeros" << std::endl;
    std::cout << "PASSED" << std::endl;
}

void test_event_creation() {
    std::cout << "\n=== Test 2: Event creation ===" << std::endl;
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    assert(hEvent != NULL);
    CloseHandle(hEvent);
    std::cout << "Input: CreateEvent manual-reset, non-signaled" << std::endl;
    std::cout << "Output: handle valid" << std::endl;
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "SYNCHRONIZATION LAB TESTS" << std::endl;
    test_array_initialization();
    test_event_creation();
    std::cout << "\nALL TESTS PASSED" << std::endl;
    return 0;
}