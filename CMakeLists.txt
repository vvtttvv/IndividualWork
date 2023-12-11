cmake_minimum_required(VERSION 3.26)
project(IW C)

set(CMAKE_C_STANDARD 11)

add_executable(IW lexer.c
        parser.c
        interpretor.c)
