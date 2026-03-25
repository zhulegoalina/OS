#define NOMINMAX
#include "sync_lab.h"
#include <cstdlib>
#include <ctime>

DWORD WINAPI marker_thread(LPVOID lpParam) {
    MarkerData* data = (MarkerData*)lpParam;

    srand(data->id);
    WaitForSingleObject(data->startEvent, INFINITE);

    int size = (int)data->array->size();
    int failCount = 0;

    while (data->active) {
        int index = rand() % size;

        EnterCriticalSection(data->cs);

        if ((*data->array)[index] == 0) {
            Sleep(5);
            (*data->array)[index] = data->id;
            Sleep(5);
            data->markedIndices.push_back(index);
            failCount = 0;
            LeaveCriticalSection(data->cs);
        }
        else {
            failCount++;
            LeaveCriticalSection(data->cs);

            if (failCount > size * 2) {
                std::cout << "Marker " << data->id
                    << " | " << data->markedIndices.size()
                    << " | blocked: no free cells" << std::endl;

                SetEvent(data->stopEvent);

                HANDLE events[2] = { data->continueEvent, data->terminateEvent };
                DWORD result = WaitForMultipleObjects(2, events, FALSE, INFINITE);

                if (result == WAIT_OBJECT_0 + 1) {
                    EnterCriticalSection(data->cs);
                    for (int idx : data->markedIndices) {
                        (*data->array)[idx] = 0;
                    }
                    LeaveCriticalSection(data->cs);
                    return 0;
                }
                else if (result == WAIT_OBJECT_0) {
                    failCount = 0;
                }
            }
        }
    }
    return 0;
}