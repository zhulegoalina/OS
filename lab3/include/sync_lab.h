#pragma once

#define NOMINMAX
#include <windows.h>
#include <vector>
#include <iostream>
#include <string>

struct MarkerData {
    int id;
    std::vector<int>* array;
    std::vector<int> markedIndices;
    HANDLE startEvent;
    HANDLE stopEvent;
    HANDLE continueEvent;
    HANDLE terminateEvent;
    CRITICAL_SECTION* cs;
    bool active;
};

DWORD WINAPI marker_thread(LPVOID lpParam);