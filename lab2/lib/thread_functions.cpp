#define NOMINMAX
#include "thread_lab.h"
#include <windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <limits>

std::string GetLastErrorAsString() {
    DWORD error = GetLastError();
    if (error == 0) return "No error";

    LPSTR messageBuffer = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer);
    LocalFree(messageBuffer);
    return message;
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool validateArray(const std::vector<int>& arr) {
    return !arr.empty();
}

void replaceMinMaxWithAverage(std::vector<int>& arr, int minVal, int maxVal, double avgVal) {
    for (size_t i = 0; i < arr.size(); i++) {
        if (arr[i] == minVal || arr[i] == maxVal) {
            arr[i] = static_cast<int>(avgVal);
        }
    }
}

DWORD WINAPI min_max_thread(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);

    try {
        if (data->array->empty()) {
            *data->errorFlag = true;
            *data->errorMessage = "Array is empty";
            return 1;
        }

        *data->min = (*data->array)[0];
        *data->max = (*data->array)[0];

        for (size_t i = 1; i < data->array->size(); i++) {
            if ((*data->array)[i] < *data->min) {
                *data->min = (*data->array)[i];
            }
            if ((*data->array)[i] > *data->max) {
                *data->max = (*data->array)[i];
            }
            Sleep(7);
        }

        return 0;
    }
    catch (const std::exception& e) {
        *data->errorFlag = true;
        *data->errorMessage = e.what();
        return 1;
    }
}

DWORD WINAPI average_thread(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);

    try {
        if (data->array->empty()) {
            *data->errorFlag = true;
            *data->errorMessage = "Array is empty";
            return 1;
        }

        long long sum = 0;

        for (size_t i = 0; i < data->array->size(); i++) {
            sum += (*data->array)[i];
            Sleep(12);
        }

        *data->average = static_cast<double>(sum) / data->array->size();

        return 0;
    }
    catch (const std::exception& e) {
        *data->errorFlag = true;
        *data->errorMessage = e.what();
        return 1;
    }
}