#include "api.h"
#include <cstdio>
#include <cstring>

int build_time_in_payload(const attendance_request_t* req,
                           char* out_buffer,
                           size_t buffer_size) {
    if (req == nullptr || out_buffer == nullptr) {
        return API_ERROR;
    }
    if (req->device_id == nullptr || req->employee_id == nullptr) {
        return API_ERROR;
    }

    time_t now = get_current_timestamp();
    char time_str[16];
    if (format_time_hhmmss(now, time_str, sizeof(time_str)) != API_SUCCESS) {
        return API_ERROR;
    }

    int written = snprintf(
        out_buffer, buffer_size,
        "{\"device_id\":\"%s\",\"employeeId\":\"%s\",\"timeIn\":\"%s\",\"timeOut\":\"\",\"timestamp\":%ld}",
        req->device_id, req->employee_id, time_str, (long)now
    );

    if (written < 0 || (size_t)written >= buffer_size) {
        return API_ERROR;
    }

    return API_SUCCESS;
}

int build_time_out_payload(const attendance_request_t* req,
                            char* out_buffer,
                            size_t buffer_size) {
    if (req == nullptr || out_buffer == nullptr) {
        return API_ERROR;
    }
    if (req->device_id == nullptr || req->employee_id == nullptr) {
        return API_ERROR;
    }

    time_t now = get_current_timestamp();
    char time_str[16];
    if (format_time_hhmmss(now, time_str, sizeof(time_str)) != API_SUCCESS) {
        return API_ERROR;
    }

    int written = snprintf(
        out_buffer, buffer_size,
        "{\"device_id\":\"%s\",\"employeeId\":\"%s\",\"timeIn\":\"\",\"timeOut\":\"%s\",\"timestamp\":%ld}",
        req->device_id, req->employee_id, time_str, (long)now
    );

    if (written < 0 || (size_t)written >= buffer_size) {
        return API_ERROR;
    }

    return API_SUCCESS;
}

time_t get_current_timestamp(void) {
    return time(nullptr);
}

int format_time_hhmmss(time_t t, char* out_buffer, size_t buffer_size) {
    if (out_buffer == nullptr || buffer_size < 9) {
        return API_ERROR;
    }

    struct tm local_tm;
    localtime_r(&t, &local_tm);

    size_t written = strftime(out_buffer, buffer_size, "%H:%M:%S", &local_tm);
    if (written == 0) {
        return API_ERROR;
    }

    return API_SUCCESS;
}