cmake_minimum_required(VERSION 3.17)
project(prj4 C)

set(CMAKE_C_STANDARD 11)

add_executable(prj4-1 "Phase1 folder/myshell.c" "Phase1 folder/csapp.c" "Phase1 folder/myshell_func.c")

add_executable(prj4-2 "Phase2 folder/myshell.c" "Phase2 folder/csapp.c" "Phase2 folder/myshell_func.c")

add_executable(prj4-3 "Phase3 folder/myshell.c" "Phase3 folder/csapp.c" "Phase3 folder/myshell_func.c")