#define NOMINMAX
#include "thread_lab.h"
#include <iostream>
#include <vector>
#include <windows.h>

int main() {
    try {
        setlocale(LC_ALL, "Russian");

        int size;
        safeInput(size, "Enter array size: ", 1, 1000000);

        std::vector<int> arr(size);

        std::cout << "Enter " << size << " integers:" << std::endl;
        for (int i = 0; i < size; i++) {
            safeInput(arr[i], "arr[" + std::to_string(i) + "] = ",
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max());
        }

        std::cout << "\nOriginal array: ";
        for (int val : arr) std::cout << val << " ";
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

        HANDLE hMinMax = CreateThread(NULL, 0, min_max_thread, &data, 0, NULL);
        if (hMinMax == NULL) {
            throw std::runtime_error("Failed to create min_max thread: " + GetLastErrorAsString());
        }

        HANDLE hAverage = CreateThread(NULL, 0, average_thread, &data, 0, NULL);
        if (hAverage == NULL) {
            CloseHandle(hMinMax);
            throw std::runtime_error("Failed to create average thread: " + GetLastErrorAsString());
        }

        std::cout << "\nWaiting for threads to finish..." << std::endl;

        WaitForSingleObject(hMinMax, INFINITE);
        WaitForSingleObject(hAverage, INFINITE);

        if (errorFlag) {
            throw std::runtime_error("Thread error: " + errorMessage);
        }

        DWORD exitCode;
        GetExitCodeThread(hMinMax, &exitCode);
        if (exitCode != 0) throw std::runtime_error("min_max thread failed");

        GetExitCodeThread(hAverage, &exitCode);
        if (exitCode != 0) throw std::runtime_error("average thread failed");

        std::cout << "\nResults:" << std::endl;
        std::cout << "Min = " << minVal << ", Max = " << maxVal << std::endl;
        std::cout << "Average = " << avgVal << std::endl;

        replaceMinMaxWithAverage(arr, minVal, maxVal, avgVal);

        std::cout << "\nModified array: ";
        for (int val : arr) std::cout << val << " ";
        std::cout << std::endl;

        CloseHandle(hMinMax);
        CloseHandle(hAverage);

        std::cout << "\nProgram completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}