cmake_minimum_required(VERSION 3.21)
project(wordcalc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")

add_executable(wordcalc main.c)
