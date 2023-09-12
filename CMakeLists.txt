cmake_minimum_required(VERSION 3.22)
project(ContextSwitcher C ASM)

set(CMAKE_C_STANDARD 11)
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")

add_executable(ContextSwitcher switch.s main.c)
target_compile_options(ContextSwitcher PRIVATE -masm=intel)