//
// 定时任务
// Created by apricot on 2024/10/8.
//

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

//单个任务
typedef struct edge_task {
    char *name; //定时任务名称
    uint32_t id; //定时任务id
    uint32_t interval; //间隔时间，毫秒值
    uint8_t railing; //是否在第一次执行时轧过00:00这种
    void (*task_func)(void); //方法
    long nextRunTime; //下一次运行时间
}edge_task_t;

//时间轮
typedef struct time_wheel {
    edge_task_t *tasks; //对象数组
    uint32_t index;//下标
}time_wheel_t;

uint32_t addTasks(char *name, const uint32_t interval, const uint8_t railing, void (*task_func)(void));


#endif //TASK_H
