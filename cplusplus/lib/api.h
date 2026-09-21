#ifndef LIB_API_H
#define LIB_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <time.h>

#define API_SUCCESS   0
#define API_ERROR    -1

typedef struct {
    const char* device_id;
    const char* employee_id;
} attendance_request_t;

int build_time_in_payload(const attendance_request_t* req,
                           char* out_buffer,
                           size_t buffer_size);

int build_time_out_payload(const attendance_request_t* req,
                            char* out_buffer,
                            size_t buffer_size);

time_t get_current_timestamp(void);

int format_time_hhmmss(time_t t, char* out_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // LIB_API_H