#include "thread_lab.h"
#include <cassert>
#include <iostream>
#include <windows.h>
#include <vector>

void test_array_validation() {
    std::cout << "\nTest 1: Array validation" << std::endl;

    std::vector<int> empty;
    std::cout << "Input: empty array" << std::endl;
    bool result = validateArray(empty);
    std::cout << "Output: " << result << " (expected: false)" << std::endl;
    assert(!result);

    std::vector<int> valid = { 1, 2, 3 };
    std::cout << "Input: [1,2,3]" << std::endl;
    result = validateArray(valid);
    std::cout << "Output: " << result << " (expected: true)" << std::endl;
    assert(result);

    std::cout << "PASSED" << std::endl;
}

void test_min_max_thread() {
    std::cout << "\nTest 2: min_max thread" << std::endl;

    std::vector<int> arr = { 5, 2, 8, 1, 9, 3 };
    std::cout << "Input array: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << std::endl;

    int minVal, maxVal;
    double avgVal;
    bool errorFlag = false;
    std::string errorMessage;

    ThreadData data;
    data.array = &arr;
    data.min = &minVal;
    data.max = &maxVal;
    data.average = &avgVal;
    data.errorFlag = &errorFlag;
    data.errorMessage = &errorMessage;

    HANDLE hThread = CreateThread(NULL, 0, min_max_thread, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "Output min: " << minVal << " (expected: 1)" << std::endl;
    std::cout << "Output max: " << maxVal << " (expected: 9)" << std::endl;

    assert(minVal == 1);
    assert(maxVal == 9);
    assert(!errorFlag);

    CloseHandle(hThread);
    std::cout << "PASSED" << std::endl;
}

void test_average_thread() {
    std::cout << "\nTest 3: average thread" << std::endl;

    std::vector<int> arr = { 10, 20, 30, 40, 50 };
    std::cout << "Input array: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << std::endl;

    int minVal, maxVal;
    double avgVal;
    bool errorFlag = false;
    std::string errorMessage;

    ThreadData data;
    data.array = &arr;
    data.min = &minVal;
    data.max = &maxVal;
    data.average = &avgVal;
    data.errorFlag = &errorFlag;
    data.errorMessage = &errorMessage;

    HANDLE hThread = CreateThread(NULL, 0, average_thread, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "Output average: " << avgVal << " (expected: 30)" << std::endl;

    assert(avgVal == 30.0);
    assert(!errorFlag);

    CloseHandle(hThread);
    std::cout << "PASSED" << std::endl;
}

void test_replace_function() {
    std::cout << "\nTest 4: replace function" << std::endl;

    std::vector<int> arr = { 1, 5, 3, 5, 2, 1 };
    std::cout << "Input array: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << std::endl;

    replaceMinMaxWithAverage(arr, 1, 5, 3.0);

    std::cout << "Output array: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << " (expected: 3 3 3 3 2 3)" << std::endl;

    std::vector<int> expected = { 3, 3, 3, 3, 2, 3 };
    assert(arr == expected);
    std::cout << "PASSED" << std::endl;
}

void test_error_handling() {
    std::cout << "\nTest 5: error handling" << std::endl;

    std::vector<int> empty;
    std::cout << "Input: empty array" << std::endl;

    int minVal, maxVal;
    double avgVal;
    bool errorFlag = false;
    std::string errorMessage;

    ThreadData data;
    data.array = &empty;
    data.min = &minVal;
    data.max = &maxVal;
    data.average = &avgVal;
    data.errorFlag = &errorFlag;
    data.errorMessage = &errorMessage;

    HANDLE hThread = CreateThread(NULL, 0, min_max_thread, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "Error flag: " << errorFlag << " (expected: true)" << std::endl;
    std::cout << "Error message: \"" << errorMessage << "\" (expected not empty)" << std::endl;

    assert(errorFlag);
    assert(!errorMessage.empty());

    CloseHandle(hThread);
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "THREAD TESTS" << std::endl;

    test_array_validation();
    test_min_max_thread();
    test_average_thread();
    test_replace_function();
    test_error_handling();

    std::cout << "\nALL TESTS PASSED" << std::endl;
    return 0;
}