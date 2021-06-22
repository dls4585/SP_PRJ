cmake_minimum_required(VERSION 3.17)
project(prj5 C)

set(CMAKE_C_STANDARD 11)

add_executable(multiclient "task1/multiclient.c" "task1/csapp.c")
add_executable(stockclient "task1/stockclient.c" "task1/csapp.c")
add_executable(stockserver "task1/stockserver.c" "task1/action.c" "task1/csapp.c")

add_executable(multiclient2 "task2/multiclient.c" "task2/csapp.c")
add_executable(stockclient2 "task2/stockclient.c" "task2/csapp.c")
add_executable(stockserver2 "task2/stockserver.c" "task2/action.c" "task2/csapp.c")

add_executable(multiclient3-1 "task3/task1/multiclient.c" "task3/task1/csapp.c")
add_executable(stockclient3-1 "task3/task1/stockclient.c" "task3/task1/csapp.c")
add_executable(stockserver3-1 "task3/task1/stockserver.c" "task3/task1/action.c" "task3/task1/csapp.c")

add_executable(multiclient3-2 "task3/task2/multiclient.c" "task3/task2/csapp.c")
add_executable(stockclient3-2 "task3/task2/stockclient.c" "task3/task2/csapp.c")
add_executable(stockserver3-2 "task3/task2/stockserver.c" "task3/task2/action.c" "task3/task2/csapp.c")
