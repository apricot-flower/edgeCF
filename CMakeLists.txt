cmake_minimum_required(VERSION 3.29)
project(edgeCF C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(edgeC src/main.c
        src/elog.c
        src/task.c)

#头文件
target_include_directories(edgeC PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)