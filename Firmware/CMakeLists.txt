cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(lib/pico_sdk_import.cmake)
include_directories(include)

project(pico506 C CXX)

pico_sdk_init()

add_compile_options(-ffunction-sections -fdata-sections -Wl,--gc-sections)

add_subdirectory(src)

pico_set_program_name(${PROJECT_NAME} "pico506")
pico_set_program_version(${PROJECT_NAME} "1.0.0")

pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
