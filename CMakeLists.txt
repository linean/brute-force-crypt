cmake_minimum_required(VERSION 3.13)
project(prir_brute_force C)

set(CMAKE_C_STANDARD 99)

add_executable(prir_brute_force main.c)
target_link_libraries(prir_brute_force crypt)
