#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <windows.h>
#include <filesystem>
#include "employee.h"

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

bool checkDiskSpace(const std::string& filename, size_t neededSize) {
    try {
        std::filesystem::space_info space = std::filesystem::space(std::filesystem::current_path());
        if (space.available < neededSize) {
            std::cerr << "Warning: Low disk space. Available: "
                << space.available / 1024 << " KB, Needed: "
                << neededSize / 1024 << " KB" << std::endl;
            std::cout << "Continue anyway? (y/n): ";
            char answer;
            std::cin >> answer;
            return (answer == 'y' || answer == 'Y');
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Warning: Cannot check disk space: " << e.what() << std::endl;
    }
    return true;
}

bool isValidFilename(const std::string& filename) {
    if (filename.empty()) {
        std::cerr << "Error: Filename cannot be empty" << std::endl;
        return false;
    }

    if (filename.length() > 255) {
        std::cerr << "Error: Filename is too long (max 255 characters)" << std::endl;
        return false;
    }

    const std::string invalidChars = "\\/:*?\"<>|";
    for (char c : filename) {
        if (invalidChars.find(c) != std::string::npos) {
            std::cerr << "Error: Filename contains invalid character: '" << c << "'" << std::endl;
            return false;
        }
    }

    std::string upperName = filename;
    for (char& c : upperName) {
        c = std::toupper(c);
    }

    std::string nameWithoutExt = upperName;
    size_t dotPos = upperName.find('.');
    if (dotPos != std::string::npos) {
        nameWithoutExt = upperName.substr(0, dotPos);
    }

    return true;
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template<typename T>
bool safeInput(T& value, const std::string& prompt) {
    std::cout << prompt;
    if (!(std::cin >> value)) {
        clearInput();
        return false;
    }
    return true;
}

HANDLE runProcess(const std::string& commandLine) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char* cmdLine = _strdup(commandLine.c_str());

    if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        free(cmdLine);
        DWORD error = GetLastError();
        throw std::runtime_error("CreateProcess failed: " + GetLastErrorAsString());
    }

    free(cmdLine);
    CloseHandle(pi.hThread);
    return pi.hProcess;
}

void displayBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        DWORD error = GetLastError();
        throw std::runtime_error("Cannot open file for display: " + filename +
            " - " + GetLastErrorAsString());
    }

    std::cout << "\nContents of " << filename << ":" << std::endl;
    std::cout << "ID\tName\tHours" << std::endl;

    employee emp;
    int count = 0;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        if (emp.isValid()) {
            std::cout << emp.num << "\t" << emp.name << "\t" << emp.hours << std::endl;
            count++;
        }
    }

    if (count == 0) {
        std::cout << "File is empty or contains no valid records." << std::endl;
    }

    file.close();
    std::cout << std::endl;
}

void displayReport(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        DWORD error = GetLastError();
        throw std::runtime_error("Cannot open report file for display: " + filename +
            " - " + GetLastErrorAsString());
    }

    std::cout << "\nReport contents:" << std::endl;

    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
        lineCount++;
    }

    if (lineCount == 0) {
        std::cout << "Report file is empty." << std::endl;
    }

    file.close();
    std::cout << std::endl;
}

int main() {
    try {
        setlocale(LC_ALL, "Russian");
        std::string binaryFilename;
        int recordCount;

        std::cout << "Enter binary filename: ";
        std::getline(std::cin, binaryFilename);

        if (!isValidFilename(binaryFilename)) {
            throw std::runtime_error("Invalid filename");
        }

        if (!checkDiskSpace(binaryFilename, sizeof(employee) * 1000)) {
            throw std::runtime_error("Insufficient disk space");
        }

        int attempts = 0;
        const int maxAttempts = 3;

        while (attempts < maxAttempts) {
            std::cout << "Enter number of records: ";
            if (std::cin >> recordCount) {
                if (recordCount > 0 && recordCount <= 10000) {
                    break;
                }
                else {
                    std::cerr << "Number of records must be between 1 and 10000!" << std::endl;
                }
            }
            else {
                std::cerr << "Invalid input. Please enter a number." << std::endl;
            }

            clearInput();
            attempts++;

            if (attempts == maxAttempts) {
                throw std::runtime_error("Too many invalid attempts. Exiting.");
            }
        }

        clearInput();

        std::string creatorCmd = "Creator.exe " + binaryFilename + " " + std::to_string(recordCount);
        std::cout << "\nStarting Creator..." << std::endl;

        HANDLE hCreator = runProcess(creatorCmd);

        std::cout << "Waiting for Creator to finish..." << std::endl;
        WaitForSingleObject(hCreator, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(hCreator, &exitCode);
        CloseHandle(hCreator);

        if (exitCode != 0) {
            throw std::runtime_error("Creator failed with exit code: " + std::to_string(exitCode));
        }

        displayBinaryFile(binaryFilename);

        std::string reportFilename;
        double hourlyRate;

        std::cout << "Enter report filename: ";
        std::getline(std::cin, reportFilename);

        if (!isValidFilename(reportFilename)) {
            throw std::runtime_error("Invalid report filename");
        }

        attempts = 0;
        while (attempts < maxAttempts) {
            std::cout << "Enter hourly rate: ";
            if (std::cin >> hourlyRate) {
                if (hourlyRate > 0 && hourlyRate <= 10000) {
                    break;
                }
                else {
                    std::cerr << "Hourly rate must be positive and <= 10000" << std::endl;
                }
            }
            else {
                std::cerr << "Invalid input. Please enter a number." << std::endl;
            }

            clearInput();
            attempts++;

            if (attempts == maxAttempts) {
                throw std::runtime_error("Too many invalid attempts. Exiting.");
            }
        }

        std::string reporterCmd = "Reporter.exe " + binaryFilename + " " +
            reportFilename + " " + std::to_string(hourlyRate);
        std::cout << "\nStarting Reporter..." << std::endl;

        HANDLE hReporter = runProcess(reporterCmd);

        std::cout << "Waiting for Reporter to finish..." << std::endl;
        WaitForSingleObject(hReporter, INFINITE);

        GetExitCodeProcess(hReporter, &exitCode);
        CloseHandle(hReporter);

        if (exitCode != 0) {
            throw std::runtime_error("Reporter failed with exit code: " + std::to_string(exitCode));
        }

        displayReport(reportFilename);

        std::cout << "Program completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "\nError in Main: " << e.what() << std::endl;
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.ignore();
        std::cin.get();
        return 1;
    }

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}