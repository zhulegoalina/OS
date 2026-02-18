#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <windows.h>
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

bool validateInput(int num, const std::string& name, double hours) {
    if (num <= 0) {
        std::cerr << "Error: ID must be positive!" << std::endl;
        return false;
    }
    if (name.empty()) {
        std::cerr << "Error: Name cannot be empty!" << std::endl;
        return false;
    }
    if (name.length() >= 10) {
        std::cerr << "Error: Name must be less than 10 characters!" << std::endl;
        return false;
    }
    if (hours < 0) {
        std::cerr << "Error: Hours cannot be negative!" << std::endl;
        return false;
    }
    return true;
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: Creator <binary_filename> <record_count>" << std::endl;
            return 1;
        }

        std::string filename = argv[1];
        int recordCount;

        try {
            recordCount = std::stoi(argv[2]);
            if (recordCount <= 0) {
                throw std::invalid_argument("Record count must be positive");
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: Invalid record count - " << e.what() << std::endl;
            return 1;
        }

        std::ifstream checkFile(filename, std::ios::binary);
        if (checkFile.good()) {
            std::cout << "File " << filename << " already exists. Overwrite? (y/n): ";
            char answer;
            std::cin >> answer;
            if (answer != 'y' && answer != 'Y') {
                std::cout << "Operation cancelled." << std::endl;
                return 0;
            }
        }
        checkFile.close();

        std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            DWORD error = GetLastError();
            throw std::runtime_error("Cannot create file: " + filename +
                " - " + GetLastErrorAsString());
        }

        std::cout << "Enter " << recordCount << " employee records:" << std::endl;
        std::cout << "Format: <id> <name> <hours>" << std::endl;

        int recordsWritten = 0;
        while (recordsWritten < recordCount) {
            std::cout << "Record " << (recordsWritten + 1) << ": ";

            int num;
            std::string name;
            double hours;

            if (!(std::cin >> num)) {
                std::cerr << "Invalid ID format. Please enter a number." << std::endl;
                clearInput();
                continue;
            }

            if (!(std::cin >> name)) {
                std::cerr << "Invalid name format." << std::endl;
                clearInput();
                continue;
            }

            if (!(std::cin >> hours)) {
                std::cerr << "Invalid hours format. Please enter a number." << std::endl;
                clearInput();
                continue;
            }

            if (!validateInput(num, name, hours)) {
                std::cout << "Please try again." << std::endl;
                continue;
            }

            employee emp(num, name, hours);
            outFile.write(reinterpret_cast<const char*>(&emp), sizeof(employee));

            if (!outFile.good()) {
                DWORD error = GetLastError();
                throw std::runtime_error("Error writing to file: " + GetLastErrorAsString());
            }

            recordsWritten++;
        }

        outFile.close();
        std::cout << "Successfully created " << filename << " with "
            << recordsWritten << " records." << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in Creator: " << e.what() << std::endl;
        return 1;
    }
}