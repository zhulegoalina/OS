#include "common.h"
#include <iostream>
#include <string>

using namespace std;

class Client {
private:
    HANDLE hPipe;
    bool connected;

    bool sendRequest(const Request& req, Response& resp) {
        DWORD bytesWritten, bytesRead;

        if (!WriteFile(hPipe, &req, sizeof(Request), &bytesWritten, NULL) || bytesWritten == 0) {
            cerr << "Failed to send request!" << endl;
            return false;
        }

        if (!ReadFile(hPipe, &resp, sizeof(Response), &bytesRead, NULL) || bytesRead == 0) {
            cerr << "Failed to read response!" << endl;
            return false;
        }

        return true;
    }

    void displayEmployee(const employee& emp) {
        cout << "ID: " << emp.num << endl;
        cout << "Name: " << emp.name << endl;
        cout << "Hours: " << emp.hours << endl;
    }

public:
    Client() : hPipe(NULL), connected(false) {}

    ~Client() {
        if (hPipe) CloseHandle(hPipe);
    }

    bool connect() {
        while (true) {
            hPipe = CreateFileA(
                PIPE_NAME,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );

            if (hPipe != INVALID_HANDLE_VALUE) {
                connected = true;
                cout << "Connected to server!" << endl;
                return true;
            }

            DWORD error = GetLastError();
            if (error != ERROR_PIPE_BUSY) {
                cerr << "Failed to connect to pipe. Error: " << error << endl;
                return false;
            }

            if (!WaitNamedPipeA(PIPE_NAME, 5000)) {
                cerr << "Timeout waiting for pipe!" << endl;
                return false;
            }
        }
    }

    void run() {
        if (!connected && !connect()) return;

        string command;
        bool running = true;

        cout << "\nClient ready!" << endl;
        cout << "Commands: 'read', 'update', 'quit'" << endl;

        while (running) {
            cout << "\n> ";
            cin >> command;

            if (command == "read") {
                int key;
                cout << "Enter employee ID to read: ";
                cin >> key;

                Request req;
                req.type = REQ_READ;
                req.key = key;

                Response resp;
                if (sendRequest(req, resp)) {
                    if (resp.status == 0) {
                        cout << "\nRecord found " << endl;
                        displayEmployee(resp.emp);
                    }
                    else {
                        cout << "Error: " << resp.message << endl;
                    }
                }
            }
            else if (command == "update") {
                int key;
                cout << "Enter employee ID to modify: ";
                cin >> key;

                Request req;
                req.type = REQ_READ_FULL;
                req.key = key;

                Response resp;
                if (!sendRequest(req, resp)) {
                    cout << "Failed to get record!" << endl;
                    continue;
                }

                if (resp.status == 2) {
                    cout << "Record is locked by another client. Try again later." << endl;
                    continue;
                }

                if (resp.status != 0) {
                    cout << "Error: " << resp.message << endl;
                    continue;
                }

                cout << "\nCurrent record" << endl;
                displayEmployee(resp.emp);

                employee newEmp;
                newEmp.num = resp.emp.num;

                cout << "Enter new name (max 9 chars): ";
                cin >> newEmp.name;

                cout << "Enter new hours: ";
                cin >> newEmp.hours;

                Request updateReq;
                updateReq.type = REQ_UPDATE;
                updateReq.key = key;
                updateReq.emp = newEmp;

                Response updateResp;
                if (sendRequest(updateReq, updateResp)) {
                    if (updateResp.status == 0) {
                        cout << "Record updated successfully!" << endl;
                    }
                    else {
                        cout << "Update failed: " << updateResp.message << endl;
                    }
                }

                Request unlockReq;
                unlockReq.type = REQ_UNLOCK;
                unlockReq.key = key;
                Response unlockResp;
                sendRequest(unlockReq, unlockResp);
            }
            else if (command == "quit") {
                running = false;
                cout << "Client terminated." << endl;
            }
            else {
                cout << "Unknown command. Use 'read', 'update', or 'quit'" << endl;
            }
        }
    }
};

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Client client;
    client.run();

    return 0;
}