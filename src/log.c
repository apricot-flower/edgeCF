//
// Created by apricot on 2024/10/2.
//

//启用 GNU C 库（glibc）提供的所有扩展功能
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#ifdef __linux__
#include <syscall.h>
#endif

#include <sys/time.h>

#ifdef WIN32

#include <windows.h>

#endif

#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif


#include <stdio.h>
#include <pthread.h>
#include "../include/log.h"

typedef struct {
    uint8_t nCurrDay;
    FILE *pHandle;
} log_file_t;

typedef struct {
    pthread_mutex_t mutex;
    log_config_t config;
    log_file_t logFile;
    uint8_t nTdSafe;
} log_t;


typedef struct {
    uint16_t nYear;
    uint8_t nMonth;
    uint8_t nDay;
    uint8_t nHour;
    uint8_t nMin;
    uint8_t nSec;
    uint16_t nUsec;
} log_date_t;

typedef struct {
    const char *pFormat;
    log_flag eFlag;
    log_date_t date;
    uint8_t nNewLine;
} log_context_t;

static log_t e_log;


static void log_sync_init(log_t *log) {
    if (!log->nTdSafe) {
        return;
    }
    pthread_mutexattr_t mutexAttr;
    //使用 pthread_mutexattr_init 初始化互斥锁属性对象。
    //使用 pthread_mutexattr_settype 设置互斥锁类型为递归互斥锁。
    //使用 pthread_mutex_init 初始化互斥锁。
    //使用 pthread_mutexattr_destroy 销毁互斥锁属性对象。
    if (pthread_mutexattr_init(&mutexAttr) || pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE) ||
        pthread_mutex_init(&log->mutex, &mutexAttr) || pthread_mutexattr_destroy(&mutexAttr)) {
        printf("<%s:%d> %s: [ERROR] Can not initialize mutex: %d\n",
               __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
}

//初始化日志
void log_init(const char *pName, uint16_t nFlags, uint8_t nTdSafe) {
    log_config_t *pCfg = &e_log.config;
    log_file_t *pFile = &e_log.logFile;
    pCfg->colorFormat = LOG_COLORING_TAG;
    pCfg->dateControl = LOG_TIME_ONLY;
    pCfg->pCallbackCtx = NULL;
    pCfg->logCallback = NULL;
    pCfg->sSeparator[0] = ' ';
    pCfg->sSeparator[1] = '\0';
    pCfg->sFilePath[0] = '.';
    pCfg->sFilePath[1] = '\0';
    pCfg->nKeepOpen = 0;
    pCfg->nTraceTid = 0;
    pCfg->nToScreen = 1;
    pCfg->nUseHeap = 1;
    pCfg->nToFile = 1;
    pCfg->nIndent = 1;
    pCfg->nRotate = 1;
    pCfg->nFlush = 1;
    pCfg->nFlags = nFlags;
    const char *pFileName = (pName != NULL) ? pName : LOG_NAME_DEFAULT;
    snprintf(pCfg->sFileName, sizeof(pCfg->sFileName), "%s", pFileName);
    pFile->pHandle = NULL;
    pFile->nCurrDay = 0;
#ifdef WIN32
    //获取标准输出句柄
    HANDLE outPut = GetStdHandle(STD_OUTPUT_HANDLE);
    //定义一个 DWORD 类型的变量 dwMode，用于存储控制台的当前模式
    DWORD mode = 0;
    //GetConsoleMode 函数接受一个控制台句柄和一个指向 DWORD 的指针，将当前模式存储在 dwMode 中
    GetConsoleMode(outPut, &mode);
    //修改控制台模式，启用处理输出和虚拟终端处理。ENABLE_PROCESSED_OUTPUT 和 ENABLE_VIRTUAL_TERMINAL_PROCESSING 是两个标志位，分别表示启用处理输出和启用虚拟终端处理
    //|= 运算符用于将这两个标志位添加到 dwMode 中
    mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(outPut, mode);
#endif
    e_log.nTdSafe = nTdSafe;
    log_sync_init(&e_log);
}


static void log_lock(log_t *log) {
    if (log->nTdSafe && pthread_mutex_lock(&log->mutex)) {
        printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n",
               __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
}

static void log_unlock(log_t *log) {
    if (log->nTdSafe && pthread_mutex_unlock(&log->mutex)) {
        printf("<%s:%d> %s: [ERROR] Can not unlock mutex: %d\n",
               __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
}

//初始化日志级别
void enable(log_flag eFlag) {
    log_lock(&e_log);
    log_config_t *pCfg = &e_log.config;
    if (eFlag == SLOG_FLAGS_ALL) pCfg->nFlags = SLOG_FLAGS_ALL;
    else if (!SLOG_FLAGS_CHECK(pCfg->nFlags, eFlag)) pCfg->nFlags |= eFlag;
    log_unlock(&e_log);
}

static uint16_t log_get_usec() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return (uint16_t) (tv.tv_usec / 1000);
}

//获取时间
static void log_get_date(log_date_t *pDate) {
    struct tm timeinfo;
    time_t rawtime = time(NULL);
#ifdef WIN32
    localtime_s(&timeinfo, &rawtime);
#else
    localtime_r(&rawtime, &timeinfo);
#endif

    pDate->nYear = timeinfo.tm_year + 1900;
    pDate->nMonth = timeinfo.tm_mon + 1;
    pDate->nDay = timeinfo.tm_mday;
    pDate->nHour = timeinfo.tm_hour;
    pDate->nMin = timeinfo.tm_min;
    pDate->nSec = timeinfo.tm_sec;
    pDate->nUsec = log_get_usec();
}


static const char *log_get_color(log_flag eFlag) {
    switch (eFlag) {
        case LOG_NOTAG:
        case LOG_NOTE:
            return LOG_EMPTY;
        case LOG_INFO:
            return LOG_COLOR_GREEN;
        case LOG_WARN:
            return LOG_COLOR_YELLOW;
        case LOG_DEBUG:
            return LOG_COLOR_BLUE;
        case LOG_ERROR:
            return LOG_COLOR_RED;
        case LOG_TRACE:
            return LOG_COLOR_CYAN;
        case LOG_FATAL:
            return LOG_COLOR_MAGENTA;
        default:
            break;
    }

    return LOG_EMPTY;
}


static int log_create_info(const log_context_t *pCtx, char *pOut, size_t nSize) {
    log_config_t *pCfg = &e_log.config;
    const log_date_t *pDate = &pCtx->date;

    char sDate[LOG_DATE_MAX + SLOG_NAME_MAX];
    sDate[0] = LOG_NUL;

    if (pCfg->dateControl == LOG_TIME_ONLY) {
        snprintf(sDate, sizeof(sDate), "%02d:%02d:%02d.%03d ",
                 pDate->nHour, pDate->nMin, pDate->nSec, pDate->nUsec);
    } else if (pCfg->dateControl == LOG_DATE_FULL) {
        snprintf(sDate, sizeof(sDate), "%04d.%02d.%02d-%02d:%02d:%02d.%03d ",
                 pDate->nYear, pDate->nMonth, pDate->nDay, pDate->nHour,
                 pDate->nMin, pDate->nSec, pDate->nUsec);
    }

    char sTid[LOG_TAG_MAX], sTag[LOG_TAG_MAX];
    uint8_t nFullColor = pCfg->colorFormat == LOG_COLORING_FULL ? 1 : 0;

    const char *pColorCode = log_get_color(pCtx->eFlag);
    const char *pColor = nFullColor ? pColorCode : LOG_EMPTY;

    log_create_tid(sTid, sizeof(sTid), pCfg->nTraceTid);
    log_create_tag(sTag, sizeof(sTag), pCtx->eFlag, pColorCode);
    return snprintf(pOut, nSize, "%s%s%s%s", pColor, sTid, sDate, sTag);
}


static void log_display_heap(const log_context_t *pCtx, va_list args) {
    size_t nBytes = 0;
    char *pMessage = NULL;
    char sLogInfo[LOG_INFO_MAX];
    nBytes += vasprintf(&pMessage, pCtx->pFormat, args);
    va_end(args);

    if (pMessage == NULL) {
        printf("<%s:%d> %s<error>%s %s: Can not allocate memory for input: errno(%d)\n",
               __FILE__, __LINE__, LOG_COLOR_RED, LOG_COLOR_RESET, __func__, errno);

        return;
    }

    int nLength = log_create_info(pCtx, sLogInfo, sizeof(sLogInfo));
    log_display_message(pCtx, sLogInfo, nLength, pMessage);
    if (pMessage != NULL) free(pMessage);
}


static void log_display_stack(const log_context_t *pCtx, va_list args) {

}

//输出日志
void log_display(log_flag eFlag, uint8_t nNewLine, char *pFormat, ...) {
    log_lock(&e_log);
    log_config_t *pCfg = &e_log.config;
    if ((SLOG_FLAGS_CHECK(e_log.config.nFlags, eFlag)) && (e_log.config.nToScreen || e_log.config.nToFile)) {
        log_context_t ctx;
        log_get_date(&ctx.date);
        ctx.eFlag = eFlag;
        ctx.pFormat = pFormat;
        ctx.nNewLine = nNewLine;
        //声明一个函数类型变量
        void (*slog_display_args)(const log_context_t *pCtx, va_list args);
        slog_display_args = pCfg->nUseHeap ? log_display_heap : log_display_stack;

    }
}
