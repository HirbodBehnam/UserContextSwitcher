cmake_minimum_required(VERSION 3.22)
project(UserContextSwitcher C ASM)

set(CMAKE_C_STANDARD 11)
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")

add_executable(UserContextSwitcher
        main.c
        switch.S
        coroutine.c)
target_compile_options(UserContextSwitcher PRIVATE -masm=intel)