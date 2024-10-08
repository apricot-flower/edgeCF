//
// Created by apricot on 2024/10/2.
//
#include <stdio.h>
#include "../include/log.h"



void logInit() {
    int nEnabledLevels = LOG_INFO | LOG_ERROR;
    nEnabledLevels |= LOG_WARN | LOG_FATAL;
    log_init("logfile", nEnabledLevels, 1);
}

int main(void) {
    logInit();
    log_warn("这是一条warn日志");
    log_info("这是一条info日志");
    log_debug("这是一条debug日志");
    return 0;
}
