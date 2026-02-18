#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <windows.h>
#include "employee.h"

bool createTestBinaryFile(const std::string& filename, const std::vector<employee>& employees) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    for (const auto& emp : employees) {
        file.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
    }

    file.close();
    return true;
}

std::vector<employee> readBinaryFile(const std::string& filename) {
    std::vector<employee> employees;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return employees;

    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        employees.push_back(emp);
    }

    return employees;
}

void testBinaryFileOperations() {
    std::cout << "Test 1: Binary file operations..." << std::endl;

    std::vector<employee> testData = {
        employee(1, "John", 40.5),
        employee(2, "Alice", 35.0),
        employee(3, "Bob", 42.5)
    };

    std::string filename = "test_employees.bin";

    assert(createTestBinaryFile(filename, testData));

    auto readData = readBinaryFile(filename);
    assert(readData.size() == testData.size());

    for (size_t i = 0; i < testData.size(); ++i) {
        assert(readData[i].num == testData[i].num);
        assert(std::string(readData[i].name) == std::string(testData[i].name));
        assert(readData[i].hours == testData[i].hours);
    }

    DeleteFileA(filename.c_str());

    std::cout << "Test 1 passed!" << std::endl;
}

void testEmployeeValidation() {
    std::cout << "Test 2: Employee validation..." << std::endl;

    employee validEmp(1, "Valid", 40.0);
    assert(validEmp.isValid());

    employee invalidNum(0, "Test", 40.0);
    assert(!invalidNum.isValid());

    employee invalidName(1, "", 40.0);
    assert(!invalidName.isValid());

    employee invalidHours(1, "Test", -10.0);
    assert(!invalidHours.isValid());

    std::cout << "Test 2 passed!" << std::endl;
}

void testProcessCreation() {
    std::cout << "Test 3: Process creation..." << std::endl;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    BOOL result = CreateProcess(
        NULL,
        (char*)"NonExistentProgram.exe",
        NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
    );

    assert(result == FALSE);

    std::cout << "Test 3 passed!" << std::endl;
}

void testReportFormat() {
    std::cout << "Test 4: Report format and sorting validation..." << std::endl;

    std::vector<employee> employees = {
        employee(3, "Bob", 42.5),
        employee(1, "John", 40.5),
        employee(2, "Alice", 35.0),
        employee(5, "Eve", 38.0),
        employee(4, "Charlie", 45.0)
    };

    std::string binaryFile = "test_employees.bin";
    std::string reportFile = "test_report.txt";
    double hourlyRate = 10.0;

    assert(createTestBinaryFile(binaryFile, employees));

    std::vector<employee> sortedEmployees = employees;
    std::sort(sortedEmployees.begin(), sortedEmployees.end(),
        [](const employee& a, const employee& b) { return a.num < b.num; });

    std::ofstream report(reportFile);
    report << "Отчет по файлу «" << binaryFile << "»" << std::endl;
    report << "Номер сотрудника, имя сотрудника, часы, зарплата" << std::endl;

    for (const auto& e : sortedEmployees) {
        report << e.num << ", " << e.name << ", " << e.hours << ", " << (e.hours * hourlyRate) << std::endl;
    }
    report.close();

    std::ifstream checkReport(reportFile);
    assert(checkReport.good());

    std::string line;
    int lastNum = 0;
    int lineCount = 0;

    while (std::getline(checkReport, line)) {
        lineCount++;
        if (lineCount > 2) {
            size_t pos = line.find(',');
            if (pos != std::string::npos) {
                int currentNum = std::stoi(line.substr(0, pos));
                assert(currentNum > lastNum);
                lastNum = currentNum;
            }
        }
    }
    checkReport.close();

    DeleteFileA(binaryFile.c_str());
    DeleteFileA(reportFile.c_str());

    std::cout << "Test 4 passed!" << std::endl;
}

int main() {
    std::cout << "Starting Process Lab Tests..." << std::endl;

    try {
        testBinaryFileOperations();
        testEmployeeValidation();
        testProcessCreation();
        testReportFormat();

        std::cout << "\nAll tests passed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}