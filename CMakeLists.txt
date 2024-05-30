
cmake_minimum_required(VERSION 3.24)

include(pico_sdk_import.cmake)
project(pico-projekt)
pico_sdk_init()



add_executable(matrix
        matrix.cpp
        )

# pull in common dependencies
target_link_libraries(matrix pico_stdlib pico_multicore hardware_adc)

pico_enable_stdio_usb(matrix 1)
pico_enable_stdio_uart(matrix 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(matrix)


