//
// Created by apricot on 2024/10/8.
//
#include "../include/task.h"

#include <stdio.h>

static time_wheel_t timeWheel;

uint32_t addTasks(char *name, const uint32_t interval, const uint8_t railing, void(*task_func)()) {
    edge_task_t *tasks = timeWheel.tasks;
    edge_task_t task = {};
    task.name = name;
    task.id = timeWheel.index;
    task.interval = interval;
    task.railing = railing;
    task.task_func = task_func;
    tasks[task.id] = task;
    if (railing == 0) {
        //无所谓指定时间结构


    }
    timeWheel.index++;
    return task.id;
}
