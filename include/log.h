//
// Created by apricot on 2024/10/2.
//

#ifndef EDGECF_LOG_H
#define EDGECF_LOG_H

#include <stdint.h>

#define LOG_FLAGS_ALL          255
#define LOG_NAME_MAX           256
#define LOG_PATH_MAX           2048
#define LOG_NAME_DEFAULT       "edge_log"
#define LOG_FLAGS_CHECK(c, f) (((c) & (f)) == (f))


#define LOG_COLOR_NORMAL       "\x1B[0m"
#define LOG_COLOR_RED          "\x1B[31m"
#define LOG_COLOR_GREEN        "\x1B[32m"
#define LOG_COLOR_YELLOW       "\x1B[33m"
#define LOG_COLOR_BLUE         "\x1B[34m"
#define LOG_COLOR_MAGENTA      "\x1B[35m"
#define LOG_COLOR_CYAN         "\x1B[36m"
#define LOG_COLOR_WHITE        "\x1B[37m"
#define LOG_COLOR_RESET        "\033[0m"


#define LOG_INFO_MAX           512

#define LOG_MESSAGE_MAX        8196
#define LOG_VERSION_MAX        128
#define LOG_PATH_MAX           2048
#define LOG_INFO_MAX           512
#define LOG_NAME_MAX           256
#define LOG_DATE_MAX           64
#define LOG_TAG_MAX            32
#define LOG_COLOR_MAX          16

#define LOG_NUL                '\0'

#define LOG_EMPTY              ""

#define LOG_INDENT             "       "

#define LOG_SPACE              " "

#define LOG_NEWLINE            "\n"



#define log_info(...) log_display(LOG_INFO, 1, __VA_ARGS__)
#define log_warn(...) log_display(LOG_WARN, 1, __VA_ARGS__)
#define log_debug(...) log_display(LOG_DEBUG, 1, __VA_ARGS__)
#define log_error(...) log_display(LOG_ERROR, 1, __VA_ARGS__)


//定义日志标志
typedef enum {
    LOG_NOTAG = (1 << 0),
    LOG_NOTE = (1 << 1),
    LOG_INFO = (1 << 2),
    LOG_WARN = (1 << 3),
    LOG_DEBUG = (1 << 4),
    LOG_TRACE = (1 << 5),
    LOG_ERROR = (1 << 6),
    LOG_FATAL = (1 << 7)
} log_flag;


//日志回调函数
// *pLog 指向日志字符串的指针
// 日志字符串的长度。这个参数提供了日志消息的字节数，方便处理二进制数据或避免字符串终止符 \0 的问题
// 日志标志。这个参数通常是一个枚举类型，表示日志的级别或类型，例如 DEBUG、INFO、WARNING、ERROR 等
//  上下文指针。这个参数可以传递任何类型的数据指针，用于在回调函数中携带额外的信息。例如，它可以指向一个结构体，该结构体包含回调函数需要的其他配置或状态信息
typedef int(*log_cb)(const char *pLog, size_t nLength, log_flag eFlag, void *pCtx);


//定义时间标志
typedef enum {
    LOG_TIME_DISABLE = 0,
    LOG_TIME_ONLY,
    LOG_DATE_FULL
} log_date;

//定义日志的颜色格式
typedef enum {
    LOG_COLORING_DISABLE = 0,
    LOG_COLORING_TAG,
    LOG_COLORING_FULL
} log_color;

//日志主体类
typedef struct {
    log_date dateControl;
    log_color colorFormat;
    log_cb logCallback;
    void *pCallbackCtx; //回调函数上下文指针。这是一个可以传递给回调函数的数据指针，用于携带额外的信息
    uint8_t nKeepOpen;//如果设置为非零值，则保持文件句柄打开以供后续的日志写入操作使用，这可以减少频繁打开和关闭文件的开销
    uint8_t nTraceTid; //如果设置为非零值，会在日志输出中包含线程ID信息，这对于多线程应用非常有用，可以帮助追踪哪个线程产生了特定的日志条目
    uint8_t nToScreen; //如果设置为非零值，表示启用屏幕输出，即日志信息会打印到终端或控制台上
    uint8_t nUseHeap; // 如果设置为非零值，表示使用动态内存分配
    uint8_t nToFile; // 如果设置为非零值，表示启用文件输出，即日志信息会被写入到一个或多个文件中
    uint8_t nIndent;//如果设置为非零值，启用日志信息的缩进，这有助于提高日志的可读性，尤其是在嵌套的函数调用或逻辑块中
    uint8_t nRotate;//如果设置为非零值，启用日志轮转功能。这意味着当达到一定的大小或条件时，旧的日志文件会被归档，并创建新的日志文件继续记录
    uint8_t nFlush;//如果设置为非零值，表示在每次屏幕日志输出后立即刷新stdout，确保日志信息及时可见
    uint16_t nFlags;//允许的日志级别标志。通过位掩码来控制哪些级别的日志信息会被记录，例如DEBUG、INFO、WARNING、ERROR等
    char sSeparator[LOG_NAME_MAX];//分隔符字符串，用于分隔日志条目中的不同部分，如日期时间、线程ID和实际的日志信息等
    char sFileName[LOG_NAME_MAX];//输出日志文件的名称
    char sFilePath[LOG_PATH_MAX];//输出日志文件的路径

} log_config_t;

//初始化日志
//pName 文件名称，指针类型常亮
//nFlags  一个 16 位的无符号整数
// uint8_t 8 位的无符号整数
void log_init(const char *pName, uint16_t nFlags, uint8_t nTdSafe);

//初始化日志级别
void enable(log_flag eFlag);

//输出日志
void log_display(log_flag eFlag, uint8_t nNewLine, char *pFormat, ...);

#endif //EDGECF_LOG_H
