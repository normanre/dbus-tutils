cmake_minimum_required(VERSION 3.14)
project(dbus-utils C)

set(CMAKE_C_STANDARD 99)

add_subdirectory("${PROJECT_SOURCE_DIR}/AccelerometerTrigger" "${PROJECT_SOURCE_DIR}/AccelerometerTrigger/output")