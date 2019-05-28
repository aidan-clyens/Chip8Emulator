#include "utils.h"

long utils_get_time_us() {
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    return (curr_time.tv_sec * (int)1e6 + curr_time.tv_usec);
}