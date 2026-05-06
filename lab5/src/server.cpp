#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <thread>
#include <mutex>

using namespace std;

class Server {
private:
    string filename;
    vector<employee> employees;
    HANDLE hPipe;
    vector<HANDLE> clientThreads;
    map<int, bool> locks;
    mutex lockMutex; 
    bool running;

    bool createBinaryFile() {
        ofstream file(filename, ios::binary | ios::trunc);
        if (!file) return false;

        for (const auto& emp : employees) {
            file.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
        }

        file.close();
        cout << "File " << filename << " created with " << employees.size() << " records" << endl;
        return true;
    }

    void printFile() {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Cannot open file: " << filename << endl;
            return;
        }

        cout << "\n File contents " << endl;
        cout << "ID\tName\tHours" << endl;

        employee emp;
        while (file.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
            if (emp.isValid()) {
                cout << emp.num << "\t" << emp.name << "\t" << emp.hours << endl;
            }
        }
        file.close();
    }

    bool findEmployee(int key, employee& emp) {
        ifstream file(filename, ios::binary);
        if (!file) return false;

        employee temp;
        while (file.read(reinterpret_cast<char*>(&temp), sizeof(employee))) {
            if (temp.isValid() && temp.num == key) {
                emp = temp;
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    bool updateEmployee(int key, const employee& newEmp) {
        fstream file(filename, ios::binary | ios::in | ios::out);
        if (!file) return false;

        streampos pos = 0;
        employee temp;
        while (file.read(reinterpret_cast<char*>(&temp), sizeof(employee))) {
            if (temp.isValid() && temp.num == key) {
                file.seekp(pos);
                file.write(reinterpret_cast<const char*>(&newEmp), sizeof(employee));
                file.close();
                return true;
            }
            pos = file.tellg();
        }
        file.close();
        return false;
    }

    bool isLocked(int key) {
        lock_guard<mutex> lock(lockMutex);
        auto it = locks.find(key);
        return (it != locks.end() && it->second);
    }

    bool lockRecord(int key) {
        lock_guard<mutex> lock(lockMutex);
        if (locks[key]) return false;
        locks[key] = true;
        return true;
    }

    void unlockRecord(int key) {
        lock_guard<mutex> lock(lockMutex);
        locks[key] = false;
        locks.erase(key);
    }

    void handleClient(HANDLE hPipeClient) {
        Request req;
        Response resp;
        DWORD bytesRead, bytesWritten;

        while (running) {
            if (!ReadFile(hPipeClient, &req, sizeof(Request), &bytesRead, NULL) || bytesRead == 0) {
                break; 
            }

            resp.status = 0;
            strcpy_s(resp.message, sizeof(resp.message), "OK");

            switch (req.type) {
            case REQ_READ:
                cout << "Client requesting READ for key: " << req.key << endl;

                if (!findEmployee(req.key, resp.emp)) {
                    resp.status = 1;
                    strcpy_s(resp.message, "Record not found");
                }
                break;

            case REQ_UPDATE:
                cout << "Client requesting UPDATE for key: " << req.key << endl;

                if (isLocked(req.key)) {
                    resp.status = 2;
                    strcpy_s(resp.message, "Record is locked by another client");
                }
                else {
                    lockRecord(req.key);

                    if (!findEmployee(req.key, resp.emp)) {
                        resp.status = 1;
                        strcpy_s(resp.message, "Record not found");
                        unlockRecord(req.key);
                    }
                }
                break;

            case REQ_READ_FULL:
                cout << "Client requesting READ (with intent to modify) for key: " << req.key << endl;

                if (isLocked(req.key)) {
                    resp.status = 2;
                    strcpy_s(resp.message, "Record is locked by another client");
                }
                else {
                    lockRecord(req.key);

                    if (!findEmployee(req.key, resp.emp)) {
                        resp.status = 1;
                        strcpy_s(resp.message, "Record not found");
                        unlockRecord(req.key);
                    }
                }
                break;

            case REQ_UNLOCK:
                cout << "Client releasing lock for key: " << req.key << endl;
                unlockRecord(req.key);
                break;

            default:
                resp.status = 1;
                strcpy_s(resp.message, "Unknown request type");
                break;
            }

            WriteFile(hPipeClient, &resp, sizeof(Response), &bytesWritten, NULL);
        }

        CloseHandle(hPipeClient);
    }

    void runServer() {
        while (running) {
            hPipe = CreateNamedPipeA(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                sizeof(Response),
                sizeof(Request),
                5000,
                NULL
            );

            if (hPipe == INVALID_HANDLE_VALUE) {
                cerr << "Failed to create named pipe. Error: " << GetLastError() << endl;
                continue;
            }

            cout << "Waiting for client connection..." << endl;

            if (!ConnectNamedPipe(hPipe, NULL)) {
                DWORD error = GetLastError();
                if (error != ERROR_PIPE_CONNECTED) {
                    cerr << "Failed to connect client. Error: " << error << endl;
                    CloseHandle(hPipe);
                    continue;
                }
            }

            cout << "Client connected!" << endl;

            HANDLE hThread = CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
                HANDLE h = (HANDLE)param;
                Server* server = (Server*)param;
                return 0;
                }, hPipe, 0, NULL);
        }
    }

public:
    Server() : hPipe(NULL), running(false) {}

    ~Server() {
        if (hPipe) CloseHandle(hPipe);
        for (auto& th : clientThreads) {
            if (th) CloseHandle(th);
        }
    }

    bool initialize() {
        int recordCount;

        cout << "Enter binary filename: ";
        cin >> filename;

        cout << "Enter number of employee records: ";
        cin >> recordCount;

        if (recordCount <= 0) {
            cerr << "Invalid record count!" << endl;
            return false;
        }

        employees.resize(recordCount);

        cout << "Enter " << recordCount << " employee records:" << endl;
        cout << "Format: <id> <name> <hours>" << endl;

        for (int i = 0; i < recordCount; i++) {
            cout << "Record " << (i + 1) << ": ";
            int num;
            string name;
            double hours;

            cin >> num >> name >> hours;

            if (num <= 0 || name.empty() || name.length() >= 10 || hours < 0) {
                cerr << "Invalid input. Please try again." << endl;
                i--;
                continue;
            }

            employees[i] = employee(num, name, hours);
        }

        if (!createBinaryFile()) {
            cerr << "Failed to create binary file!" << endl;
            return false;
        }

        return true;
    }

    void displayFile() {
        printFile();
    }

    bool startClients(int clientCount) {
        cout << "\nEnter number of client processes: ";
        cin >> clientCount;

        if (clientCount <= 0) return false;

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        string path(exePath);
        size_t pos = path.find_last_of("\\/");
        string clientPath = path.substr(0, pos + 1) + "client.exe";

        for (int i = 0; i < clientCount; i++) {
            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            string cmdLine = clientPath;

            char* cmdLineStr = new char[cmdLine.size() + 1];
            strcpy(cmdLineStr, cmdLine.c_str());

            if (!CreateProcessA(clientPath.c_str(), cmdLineStr,
                NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                cerr << "Failed to start client " << i << endl;
                delete[] cmdLineStr;
                return false;
            }

            delete[] cmdLineStr;
            CloseHandle(pi.hThread);
        }

        cout << "Started " << clientCount << " client processes" << endl;
        return true;
    }

    void serveRequests() {
        running = true;

        thread serverThread([this]() {
            while (running) {
                HANDLE hPipe = CreateNamedPipeA(
                    PIPE_NAME,
                    PIPE_ACCESS_DUPLEX,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                    PIPE_UNLIMITED_INSTANCES,
                    sizeof(Response),
                    sizeof(Request),
                    5000,
                    NULL
                );

                if (hPipe == INVALID_HANDLE_VALUE) {
                    cerr << "CreateNamedPipe failed: " << GetLastError() << endl;
                    continue;
                }

                cout << "Waiting for client connection..." << endl;

                if (!ConnectNamedPipe(hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED) {
                    cerr << "ConnectNamedPipe failed: " << GetLastError() << endl;
                    CloseHandle(hPipe);
                    continue;
                }

                cout << "Client connected!" << endl;

                HANDLE hThread = CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
                    HANDLE hPipeClient = (HANDLE)param;
                    Server* server = (Server*)((void**)param)[1];
                    return 0;
                    }, hPipe, 0, NULL);
            }
            });
    }

    void displayModifiedFile() {
        cout << "\nAll clients completed." << endl;
        printFile();
    }

    void waitForExit() {
        cout << "\nEnter 'quit' to exit server: ";
        string cmd;
        cin >> cmd;
        if (cmd == "quit") {
            running = false;
        }
    }
};

DWORD WINAPI clientHandler(LPVOID param) {
    HANDLE hPipe = (HANDLE)param;
    Server* server = (Server*)((void**)param)[1];
    return 0;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Server server;

    if (!server.initialize()) {
        cerr << "Initialization failed!" << endl;
        return 1;
    }

    server.displayFile();

    int clientCount;
    if (!server.startClients(clientCount)) {
        cerr << "Failed to start clients!" << endl;
        return 1;
    }

    cout << "Server is running. Press Enter to stop and display final file..." << endl;
    cin.get();
    cin.get();

    server.displayModifiedFile();

    return 0;
}