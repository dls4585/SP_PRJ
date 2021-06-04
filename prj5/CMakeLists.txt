cmake_minimum_required(VERSION 3.17)
project(prj5 C)

set(CMAKE_C_STANDARD 11)

add_executable(multiclient "task1/multiclient.c" "task1/csapp.c")
add_executable(stockclient "task1/stockclient.c" "task1/csapp.c")
add_executable(stockserver "task1/stockserver.c" "task1/echo.c" "task1/csapp.c")
