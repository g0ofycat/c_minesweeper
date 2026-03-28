cmake_minimum_required(VERSION 3.12)
project(minesweeper C)

add_executable(minesweeper
    testing/main.c
    src/minesweeper_registry.c
)
