#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <string>
#include <cstring>

#define MAX_MESSAGE_LEN 20
#define MAX_PATH_LEN 260

struct Message {
    char data[MAX_MESSAGE_LEN + 1];
    BOOL isValid;

    Message() : isValid(FALSE) {
        memset(data, 0, sizeof(data));
    }
};

#pragma pack(push, 1)
struct QueueHeader {
    int readIndex;
    int writeIndex;
    int count;
    int capacity;
    int nextMsgId;

    QueueHeader() : readIndex(0), writeIndex(0), count(0), capacity(0), nextMsgId(0) {}
};
#pragma pack(pop)

const char* READY_EVENT_PREFIX = "Global\\ReadyEvent_";
const char* MUTEX_NAME = "Global\\QueueMutex";
const char* DATA_AVAILABLE_EVENT = "Global\\DataAvailable";
const char* SPACE_AVAILABLE_EVENT = "Global\\SpaceAvailable";

#endif