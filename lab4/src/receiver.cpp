#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

class Receiver {
private:
    string filename;
    int capacity;
    HANDLE hMutex;
    HANDLE hDataAvailable;
    HANDLE hSpaceAvailable;
    vector<HANDLE> readyEvents;
    vector<PROCESS_INFORMATION> senderProcesses;

    bool CreateQueueFile() {
        ofstream file(filename, ios::binary | ios::trunc);
        if (!file) return false;

        QueueHeader header;
        header.capacity = capacity;
        file.write(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        Message emptyMsg;
        for (int i = 0; i < capacity; i++) {
            file.write(reinterpret_cast<char*>(&emptyMsg), sizeof(Message));
        }

        file.close();
        return true;
    }

    bool ReadMessage(string& message, int& msgId) {
        WaitForSingleObject(hMutex, INFINITE);

        fstream file(filename, ios::binary | ios::in | ios::out);
        if (!file) {
            ReleaseMutex(hMutex);
            return false;
        }

        QueueHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        if (header.count == 0) {
            file.close();
            ReleaseMutex(hMutex);
            return false;
        }

        streampos msgPos = sizeof(QueueHeader) + header.readIndex * sizeof(Message);
        file.seekg(msgPos);

        Message msg;
        file.read(reinterpret_cast<char*>(&msg), sizeof(Message));

        msg.isValid = FALSE;
        file.seekp(msgPos);
        file.write(reinterpret_cast<char*>(&msg), sizeof(Message));

        msgId = header.nextMsgId - header.count + 1;
        header.readIndex = (header.readIndex + 1) % header.capacity;
        header.count--;

        file.seekp(0);
        file.write(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        file.close();
        message = msg.data;

        ReleaseMutex(hMutex);

        SetEvent(hSpaceAvailable);

        return true;
    }

    bool WaitForAllSenders() {
        cout << "Waiting for all senders to be ready..." << endl;
        DWORD result = WaitForMultipleObjects(readyEvents.size(),
            readyEvents.data(),
            TRUE,
            INFINITE);
        return result != WAIT_FAILED;
    }

public:
    Receiver() : hMutex(NULL), hDataAvailable(NULL), hSpaceAvailable(NULL) {}

    ~Receiver() {
        if (hMutex) CloseHandle(hMutex);
        if (hDataAvailable) CloseHandle(hDataAvailable);
        if (hSpaceAvailable) CloseHandle(hSpaceAvailable);
        for (auto& event : readyEvents) CloseHandle(event);
        for (auto& pi : senderProcesses) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    bool Initialize() {
        cout << "Enter binary filename: ";
        cin >> filename;

        cout << "Enter number of records (queue capacity): ";
        cin >> capacity;

        if (capacity <= 0 || capacity > 1000) {
            cerr << "Invalid capacity!" << endl;
            return false;
        }

        hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
        hDataAvailable = CreateEventA(NULL, FALSE, FALSE, DATA_AVAILABLE_EVENT);
        hSpaceAvailable = CreateEventA(NULL, FALSE, TRUE, SPACE_AVAILABLE_EVENT);

        if (!hMutex || !hDataAvailable || !hSpaceAvailable) {
            cerr << "Failed to create sync objects!" << endl;
            return false;
        }

        if (!CreateQueueFile()) {
            cerr << "Failed to create queue file!" << endl;
            return false;
        }

        return true;
    }

    bool StartSenders(int numSenders) {
        cout << "Enter number of sender processes: ";
        cin >> numSenders;

        if (numSenders <= 0) return false;

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        string path(exePath);
        size_t pos = path.find_last_of("\\/");
        string senderPath = path.substr(0, pos + 1) + "sender.exe";

        for (int i = 0; i < numSenders; i++) {
            string eventName = READY_EVENT_PREFIX + to_string(i);
            HANDLE hReadyEvent = CreateEventA(NULL, FALSE, FALSE, eventName.c_str());
            readyEvents.push_back(hReadyEvent);

            string cmdLine = senderPath + " " + filename + " " + to_string(i) + " " + eventName;

            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi;

            char* cmdLineStr = new char[cmdLine.size() + 1];
            strcpy(cmdLineStr, cmdLine.c_str());

            if (!CreateProcessA(senderPath.c_str(),
                cmdLineStr,
                NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                cerr << "Failed to create sender process " << i << endl;
                delete[] cmdLineStr;
                return false;
            }

            delete[] cmdLineStr;
            senderProcesses.push_back(pi);
            cout << "Started sender " << i << " with PID: " << pi.dwProcessId << endl;
        }

        return WaitForAllSenders();
    }

    void Run() {
        string command;
        bool running = true;

        cout << "\nReceiver Ready" << endl;
        cout << "Commands: 'read' - read message, 'quit' - exit" << endl;

        while (running) {
            cout << "\n> ";
            cin >> command;

            if (command == "read") {
                cout << "Waiting for message..." << endl;

                WaitForSingleObject(hDataAvailable, INFINITE);

                string message;
                int msgId;
                if (ReadMessage(message, msgId)) {
                    cout << "Received [" << msgId << "]: " << message << endl;
                }
            }
            else if (command == "quit") {
                running = false;
                cout << "Shutting down..." << endl;

                for (auto& pi : senderProcesses) {
                    TerminateProcess(pi.hProcess, 0);
                }
            }
            else {
                cout << "Unknown command. Use 'read' or 'quit'" << endl;
            }
        }
    }
};

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Receiver receiver;

    if (!receiver.Initialize()) {
        cerr << "Initialization failed!" << endl;
        return 1;
    }

    if (!receiver.StartSenders(0)) {
        cerr << "Failed to start senders!" << endl;
        return 1;
    }

    receiver.Run();

    return 0;
}