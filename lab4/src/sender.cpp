#include "common.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Sender {
private:
    string filename;
    int senderId;
    HANDLE hMutex;
    HANDLE hDataAvailable;
    HANDLE hSpaceAvailable;
    HANDLE hReadyEvent;

    bool SendMessage(const string& text) {
        WaitForSingleObject(hSpaceAvailable, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        fstream file(filename, ios::binary | ios::in | ios::out);
        if (!file) {
            ReleaseMutex(hMutex);
            return false;
        }

        QueueHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        if (header.count >= header.capacity) {
            file.close();
            ReleaseMutex(hMutex);
            SetEvent(hSpaceAvailable);
            return false;
        }

        Message msg;
        strcpy_s(msg.data, MAX_MESSAGE_LEN + 1, text.c_str());
        msg.isValid = TRUE;

        streampos msgPos = sizeof(QueueHeader) + header.writeIndex * sizeof(Message);
        file.seekp(msgPos);
        file.write(reinterpret_cast<char*>(&msg), sizeof(Message));

        header.writeIndex = (header.writeIndex + 1) % header.capacity;
        header.count++;
        header.nextMsgId++;

        file.seekp(0);
        file.write(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        file.close();

        ReleaseMutex(hMutex);

        SetEvent(hDataAvailable);

        cout << "Sender " << senderId << " sent: " << text << endl;
        return true;
    }

public:
    Sender() : hMutex(NULL), hDataAvailable(NULL),
        hSpaceAvailable(NULL), hReadyEvent(NULL) {
    }

    bool Initialize(int argc, char* argv[]) {
        if (argc != 4) {
            cerr << "Usage: sender.exe <filename> <senderId> <readyEventName>" << endl;
            return false;
        }

        filename = argv[1];
        senderId = atoi(argv[2]);
        string readyEventName = argv[3];

        hMutex = OpenMutexA(SYNCHRONIZE, FALSE, MUTEX_NAME);
        hDataAvailable = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, DATA_AVAILABLE_EVENT);
        hSpaceAvailable = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, SPACE_AVAILABLE_EVENT);

        hReadyEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, readyEventName.c_str());

        if (!hMutex || !hDataAvailable || !hSpaceAvailable || !hReadyEvent) {
            cerr << "Failed to open sync objects! Error: " << GetLastError() << endl;
            return false;
        }

        return true;
    }

    void SignalReady() {
        SetEvent(hReadyEvent);
        cout << "Sender " << senderId << " is ready!" << endl;
    }

    void Run() {
        string command;
        bool running = true;

        cout << "\nSender " << senderId << " - Commands: 'send' or 'quit'" << endl;

        while (running) {
            cout << "[" << senderId << "]> ";
            cin >> command;

            if (command == "send") {
                string message;
                cout << "Enter message (max " << MAX_MESSAGE_LEN << " chars): ";
                cin.ignore();
                getline(cin, message);

                if (message.length() > MAX_MESSAGE_LEN) {
                    cout << "Message too long! Truncating..." << endl;
                    message = message.substr(0, MAX_MESSAGE_LEN);
                }

                if (!SendMessage(message)) {
                    cout << "Failed to send message!" << endl;
                }
            }
            else if (command == "quit") {
                running = false;
                cout << "Sender " << senderId << " terminated." << endl;
            }
            else {
                cout << "Unknown command. Use 'send' or 'quit'" << endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Sender sender;

    if (!sender.Initialize(argc, argv)) {
        cerr << "Sender initialization failed!" << endl;
        return 1;
    }

    sender.SignalReady();
    sender.Run();

    return 0;
}