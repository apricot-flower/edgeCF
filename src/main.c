//
// Created by apricot on 2024/10/2.
//
#include <stdio.h>
#include "../include/elog.h"
#include "../include/task.h"



void logInit() {
    log_init("edge_logs", 7);
}

void task_test() {
    printf("task_test\n");
}

void add_task() {
    addTasks("task_test", 60 * 1000, 0, task_test);
}

int main(void) {
    logInit();
    log_info("111111111 %s", "tuluo");
    log_debug("debug %s", "tuluo");
    log_warn("warn %s", "tuluo");
    log_error("error %s", "tuluo");
    add_task();
    return 0;
}
