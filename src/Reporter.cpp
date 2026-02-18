#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
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

bool compareEmployees(const employee& a, const employee& b) {
    return a.num < b.num;
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage: Reporter <binary_file> <report_file> <hourly_rate>" << std::endl;
            return 1;
        }

        std::string binaryFilename = argv[1];
        std::string reportFilename = argv[2];
        double hourlyRate;

        try {
            hourlyRate = std::stod(argv[3]);
            if (hourlyRate <= 0) {
                throw std::invalid_argument("Hourly rate must be positive");
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: Invalid hourly rate - " << e.what() << std::endl;
            return 1;
        }

        std::ifstream inFile(binaryFilename, std::ios::binary);
        if (!inFile.is_open()) {
            DWORD error = GetLastError();
            throw std::runtime_error("Cannot open binary file: " + binaryFilename +
                " - " + GetLastErrorAsString());
        }

        std::vector<employee> employees;
        employee emp;

        while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
            if (emp.isValid()) {
                employees.push_back(emp);
            }
        }

        inFile.close();

        if (employees.empty()) {
            throw std::runtime_error("No valid records found in binary file");
        }

        std::sort(employees.begin(), employees.end(), compareEmployees);

        std::ofstream reportFile(reportFilename);
        if (!reportFile.is_open()) {
            DWORD error = GetLastError();
            throw std::runtime_error("Cannot create report file: " + reportFilename +
                " - " + GetLastErrorAsString());
        }

        reportFile << "Отчет по файлу «" << binaryFilename << "» :" << std::endl;
        reportFile << "Номер сотрудника, имя сотрудника, часы, зарплата" << std::endl;

        for (const auto& e : employees) {
            double salary = e.hours * hourlyRate;
            reportFile << e.num << ", " << e.name << ", " << e.hours << ", " << salary << std::endl;
        }

        reportFile.close();
        std::cout << "Report successfully created: " << reportFilename << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in Reporter: " << e.what() << std::endl;
        return 1;
    }
}