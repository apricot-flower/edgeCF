//
// Created by apricot on 2024/10/8.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/elog.h"

#include <stdarg.h>
#include <pthread.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define SPLIT_SYMBOL '\\'
#define LOG_FOLDER_PATH "C:\\edge_logs"
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#define SPLIT_SYMBOL '/'
#define LOG_FOLDER_PATH "/usr/local"
#endif

typedef struct {
    log_config_t config; //日志配置
}log_context_t;

static log_context_t log_context;
static pthread_mutex_t lock;

static char* get_now_time() {
    static char buffer[80];
    time_t raw_time;
    // 获取当前时间
    time(&raw_time);
    const struct tm *time_info = localtime(&raw_time);
    // 格式化时间
    snprintf(buffer, sizeof(buffer), "%d-%d-%d %d:%d:%d",
             time_info->tm_year + 1900, time_info->tm_mon + 1,time_info->tm_mday,
             time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

    return buffer;
}

static char* get_log_folder_path() {
    static char *folder[1024];
    if (getcwd(folder, sizeof(folder)) != NULL) {
        // 拼接当前工作目录
        snprintf(folder, sizeof(folder), "%s%c%s", &folder, SPLIT_SYMBOL, LOG_FOLDER);
    } else {
        snprintf(folder, sizeof(folder), "%s%c%s", &folder, LOG_FOLDER_PATH,SPLIT_SYMBOL, LOG_FOLDER);
    }
    return folder;
}

static void check_log_folder() {
    const char *folder = get_log_folder_path();
    if (access(folder, F_OK) == -1) {
        mkdir(folder);
    }
}

//校验文件是否存在
static char* check_log_file() {
    //生成今天的日志文件名称
    static char *buffer[80];
    time_t raw_time;
    // 获取当前时间
    time(&raw_time);
    struct tm *time_info = localtime(&raw_time);
    // 格式化时间
    strftime(buffer, sizeof(buffer), "%Y_%m_%d", time_info);
    static char *folder[1024];
    if (getcwd(folder, sizeof(folder)) != NULL) {
        // 拼接当前工作目录
        snprintf(folder, sizeof(folder), "%s%c%s%c%s%s", &folder, SPLIT_SYMBOL, LOG_FOLDER, SPLIT_SYMBOL, buffer, ".log");
    } else {
        snprintf(folder, sizeof(folder), "%s%c%s%c%s%s", &folder, LOG_FOLDER_PATH,SPLIT_SYMBOL, LOG_FOLDER, SPLIT_SYMBOL, buffer, ".log");
    }
    return folder;
}

//生成日志
void log(const log_level level, const char *file, const int line, char *param, ...) {
    pthread_mutex_lock(&lock);
    char *log_items[2028];
    char *log_flag = "UNKNOWN";
    switch (level) {
        case DEBUG:
            log_flag = "DEBUG";
            break;
        case INFO:
            log_flag = "INFO";
            break;
        case WARN:
            log_flag = "WARN";
            break;
        case ERR:
            log_flag = "ERROR";
            break;
        default:
            break;
    }
    snprintf(log_items, sizeof(log_items), "%s%s%s%s%s%s%d%s", get_now_time(), LOG_INTERVAL, log_flag, LOG_INTERVAL, file, ":", line, LOG_INTERVAL);
    char *log_info[2048];
    va_list ap;
    va_start(ap, param);
    vsprintf(log_info, param, ap);
    va_end(ap);
    snprintf(log_items, sizeof(log_items), "%s%s\n", log_items, log_info);
    //保存日志
    const char *log_file_path = check_log_file();
    FILE *fptr = fopen(log_file_path, "a");
    if (fptr == NULL) {
        printf("Error opening log file %s\n", log_file_path);
        pthread_mutex_unlock(&lock);
        return;
    }
    fprintf(fptr, log_items);
    fflush(fptr);
    fclose(fptr);
    pthread_mutex_unlock(&lock);
}

//删除过期的日志
static void remove_expire_log() {
    const log_config_t *log_config = &log_context.config;
    const uint8_t expire_flag = log_config->dayFlag;
    // 获取当前时间
    time_t now;
    time(&now);
    // 将当前时间转换为本地时间
    struct tm *local = localtime(&now);
    // 减去7天
    local->tm_mday -= expire_flag;
    // 调整日期，确保日期有效
    mktime(local);
    // 定义日期格式
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y_%m_%d", local);
    //遍历日志文件夹下面的文件
    const char *log_folder_path = get_log_folder_path();
    printf("%s\n", buffer);

}



//初始化日志
void log_init(char *fileName, const uint8_t dayFlag) {
    log_config_t *log_config = &log_context.config;
    log_config->fileName = fileName;
    log_config->dayFlag = dayFlag;
    check_log_folder();
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Error initializing log mutex\n");
        exit(EXIT_FAILURE);
    }
    remove_expire_log();
}
