//
// Created by apricot on 2024/10/8.
//

#ifndef ELOG_H
#define ELOG_H

#define LOG_FOLDER "edge_logs"
#define LOG_INTERVAL "  "

#define log_debug(...) log(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log(INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log(WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log(ERR, __FILE__, __LINE__, __VA_ARGS__)

#include <stdint.h>

typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERR = 3
}log_level;

typedef struct {
    char *fileName; //使用字符指针声明日志文件名
    uint8_t dayFlag; //日志保存多少天
}log_config_t;

void log(log_level level, const char *file, const int line, char *param, ...);


void log_init(char *fileName, const uint8_t dayFlag);

#endif //ELOG_H
