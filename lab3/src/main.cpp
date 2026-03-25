#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

struct MarkerData {
    int id;
    std::vector<int>* arr;
    std::vector<int> marked;
    HANDLE startEvent;
    HANDLE stopEvent;
    HANDLE continueEvent;
    HANDLE terminateEvent;
    CRITICAL_SECTION* cs;
    bool active;
};

DWORD WINAPI Marker(LPVOID p) {
    MarkerData* d = (MarkerData*)p;
    srand(d->id);
    WaitForSingleObject(d->startEvent, INFINITE);
    int size = (int)d->arr->size();

    while (d->active) {
        int idx = rand() % size;

        EnterCriticalSection(d->cs);
        if ((*d->arr)[idx] == 0) {
            Sleep(5);
            (*d->arr)[idx] = d->id;
            Sleep(5);
            d->marked.push_back(idx);
            LeaveCriticalSection(d->cs);
        }
        else {
            std::cout << "Marker " << d->id << " | " << d->marked.size() << " | " << idx << std::endl;
            LeaveCriticalSection(d->cs);
            SetEvent(d->stopEvent);

            HANDLE ev[2] = { d->continueEvent, d->terminateEvent };
            DWORD res = WaitForMultipleObjects(2, ev, FALSE, INFINITE);

            if (res == WAIT_OBJECT_0 + 1) {
                EnterCriticalSection(d->cs);
                for (int i : d->marked) (*d->arr)[i] = 0;
                LeaveCriticalSection(d->cs);
                return 0;
            }
        }
    }
    return 0;
}

int main() {
    int size, n;
    std::cout << "Size: ";
    std::cin >> size;
    std::cout << "Markers: ";
    std::cin >> n;

    std::vector<int> arr(size, 0);
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    HANDLE start = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE cont = CreateEvent(NULL, FALSE, FALSE, NULL);

    std::vector<HANDLE> threads(n);
    std::vector<HANDLE> stop(n);
    std::vector<HANDLE> term(n);
    std::vector<MarkerData> data(n);
    std::vector<bool> active(n, true);

    for (int i = 0; i < n; i++) {
        stop[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        term[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        data[i].id = i + 1;
        data[i].arr = &arr;
        data[i].startEvent = start;
        data[i].stopEvent = stop[i];
        data[i].continueEvent = cont;
        data[i].terminateEvent = term[i];
        data[i].cs = &cs;
        data[i].active = true;
        threads[i] = CreateThread(NULL, 0, Marker, &data[i], 0, NULL);
    }

    SetEvent(start);

    int left = n;

    std::vector<HANDLE> firstWait;
    for (int i = 0; i < n; i++) firstWait.push_back(stop[i]);

    std::cout << "\nWaiting for all markers to block...\n";
    WaitForMultipleObjects(n, firstWait.data(), TRUE, INFINITE);

    std::cout << "Array: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << std::endl;

    for (int k = 0; k < n; k++) {
        int kill;
        if (k == 0) {
            std::cout << "Enter marker number to terminate: ";
            std::cin >> kill;
            kill--;
        }
        else {
            kill = k;
            std::cout << "Terminating marker " << kill + 1 << " automatically..." << std::endl;
        }

        active[kill] = false;
        SetEvent(term[kill]);
        WaitForSingleObject(threads[kill], INFINITE);
        CloseHandle(threads[kill]);

        std::cout << "Array after termination: ";
        for (int v : arr) std::cout << v << " ";
        std::cout << std::endl;

        if (k < n - 1) {
            SetEvent(cont);
        }
    }

    DeleteCriticalSection(&cs);
    std::cout << "Done\n";
    return 0;
}