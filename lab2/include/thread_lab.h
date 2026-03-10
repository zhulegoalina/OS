#pragma once

#include <windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <limits>
#include <exception>

struct ThreadData {
    std::vector<int>* array;
    int* min;
    int* max;
    double* average;
    bool* errorFlag; 
    std::string* errorMessage;
};

bool validateArray(const std::vector<int>& arr);
void replaceMinMaxWithAverage(std::vector<int>& arr, int minVal, int maxVal, double avgVal);

DWORD WINAPI min_max_thread(LPVOID lpParam);
DWORD WINAPI average_thread(LPVOID lpParam);

std::string GetLastErrorAsString();
void clearInput();
template<typename T>
bool safeInput(T& value, const std::string& prompt, T min, T max);

template<typename T>
bool safeInput(T& value, const std::string& prompt, T min, T max) {
    int attempts = 0;
    const int maxAttempts = 3;

    while (attempts < maxAttempts) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (value >= min && value <= max) {
                return true;
            }
            else {
                std::cerr << "Value must be between " << min << " and " << max << std::endl;
            }
        }
        else {
            std::cerr << "Invalid input. Please enter a number." << std::endl;
        }

        clearInput();
        attempts++;
    }

    throw std::runtime_error("Too many invalid attempts");
}