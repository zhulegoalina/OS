#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <string>
#include <cstring>

#define PIPE_NAME "\\\\.\\pipe\\EmployeePipe"
#define MAX_MESSAGE_SIZE 256 

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

enum RequestType {
    REQ_READ = 1, 
    REQ_UPDATE = 2, 
    REQ_LOCK = 3, 
    REQ_UNLOCK = 4,
    REQ_READ_FULL = 5
};

struct Request {
    DWORD type;
    int key; 
    employee emp;
};

struct Response {
    DWORD status;
    employee emp;
    char message[128];
};

#endif