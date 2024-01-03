cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(iicky_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(iicky
  iicky.c
)
pico_enable_stdio_usb(iicky 1)
pico_enable_stdio_uart(iicky 0)
pico_add_extra_outputs(iicky)
target_link_libraries(iicky pico_stdlib hardware_i2c)
