#include "lib/api.h"
#include <cassert>
#include <cstring>
#include <iostream>

void test_build_time_in_payload_success() {
    attendance_request_t req = { "esp32-01", "EMP-1024" };
    char buffer[256];

    int result = build_time_in_payload(&req, buffer, sizeof(buffer));
    assert(result == API_SUCCESS);
    assert(strstr(buffer, "\"employeeId\":\"EMP-1024\"") != nullptr);
    assert(strstr(buffer, "\"timeOut\":\"\"") != nullptr);
    std::cout << "test_build_time_in_payload_success PASSED\n";
}

void test_build_payload_null_request_fails() {
    char buffer[256];
    int result = build_time_in_payload(nullptr, buffer, sizeof(buffer));
    assert(result == API_ERROR);
    std::cout << "test_build_payload_null_request_fails PASSED\n";
}

void test_build_payload_tiny_buffer_fails() {
    attendance_request_t req = { "esp32-01", "EMP-1026" };
    char tiny_buffer[5];
    int result = build_time_in_payload(&req, tiny_buffer, sizeof(tiny_buffer));
    assert(result == API_ERROR);
    std::cout << "test_build_payload_tiny_buffer_fails PASSED\n";
}

int main() {
    test_build_time_in_payload_success();
    test_build_payload_null_request_fails();
    test_build_payload_tiny_buffer_fails();
    std::cout << "All api tests passed.\n";
    return 0;
}