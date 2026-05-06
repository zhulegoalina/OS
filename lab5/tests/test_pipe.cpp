#include "common.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace std;

void test_employee_structure() {
    cout << "\nTest 1: Employee structure" << endl;

    employee e1(1, "John", 40.5);
    assert(e1.num == 1);
    assert(string(e1.name) == "John");
    assert(e1.hours == 40.5);
    assert(e1.isValid());

    employee e2(0, "Test", 40.0);
    assert(!e2.isValid());

    employee e3(1, "", 40.0);
    assert(!e3.isValid());

    employee e4(1, "Test", -10.0);
    assert(!e4.isValid());

    cout << "Input: employee(1, 'John', 40.5)" << endl;
    cout << "Output: num=1, name=John, hours=40.5" << endl;
    cout << "PASSED" << endl;
}

void test_request_response_structures() {
    cout << "\nTest 2: Request/Response structures" << endl;

    Request req;
    req.type = REQ_READ;
    req.key = 123;
    req.emp = employee(123, "Test", 40.0);

    assert(req.type == REQ_READ);
    assert(req.key == 123);
    assert(req.emp.num == 123);

    Response resp;
    resp.status = 0;
    strcpy_s(resp.message, "OK");
    resp.emp = employee(1, "John", 40.5);

    assert(resp.status == 0);
    assert(string(resp.message) == "OK");

    cout << "Input: REQ_READ, key=123" << endl;
    cout << "Output: Request structure works" << endl;
    cout << "PASSED" << endl;
}

void test_pipe_name() {
    cout << "\nTest 3: Pipe name" << endl;

    string pipeName = PIPE_NAME;
    string expected = "\\\\.\\pipe\\EmployeePipe";

    assert(pipeName == expected);

    cout << "Input: " << expected << endl;
    cout << "Output: " << pipeName << endl;
    cout << "PASSED" << endl;
}

int main() {
    cout << "   NAMED PIPE LABORATORY TESTS" << endl;

    test_employee_structure();
    test_request_response_structures();
    test_pipe_name();

    cout << "   ALL TESTS PASSED!" << endl;

    return 0;
}