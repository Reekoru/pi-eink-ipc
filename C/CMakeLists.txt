cmake_minimum_required(VERSION 3.13)
project(socket_test_cjson C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(socket_test
    socket_test.c
    utils/cJSON.c
    json_types.c
    utils/message.c
)

if (MSVC)
    target_compile_options(socket_test PRIVATE /W4)
else()
    target_compile_options(socket_test PRIVATE -Wall -Wextra -O2)
endif()
