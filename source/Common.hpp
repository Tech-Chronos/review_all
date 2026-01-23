#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <iostream>
#include <ctime>
#include <pthread.h>

#define LOG_INF 0
#define LOG_DBG 1
#define LOG_ERR 2

#define LOG(level, format, ...)                                                            \
    do                                                                                     \
    {                                                                                      \
        time_t curtime = time(nullptr);                                                    \
        struct tm *local = localtime(&curtime);                                            \
        char tmp[32] = {0};                                                                \
        strftime(tmp, 31, "%H:%M:%S", local);                                              \
        fprintf(stdout, "[%p %s][%s:%d]" format "\n", (void*)pthread_self(), tmp, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define INF_LOG(format, ...) LOG(LOG_INF, format, ##__VA_ARGS__)
#define DBG_LOG(format, ...) LOG(LOG_DBG, format, ##__VA_ARGS__)
#define ERR_LOG(format, ...) LOG(LOG_ERR, format, ##__VA_ARGS__)

#endif