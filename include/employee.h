#pragma once

#include <cstring>
#include <string>

struct employee {
    int num;
    char name[10];
    double hours;

    employee() : num(0), hours(0.0) {
        memset(name, 0, sizeof(name));
    }

    employee(int id, const std::string& empName, double workHours) 
        : num(id), hours(workHours) {
        memset(name, 0, sizeof(name));
        strncpy_s(name, sizeof(name), empName.c_str(), _TRUNCATE);
    }

    bool isValid() const {
        return num > 0 && hours >= 0 && name[0] != '\0';
    }
};