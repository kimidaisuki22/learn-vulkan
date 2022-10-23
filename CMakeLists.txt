cmake_minimum_required(VERSION 3.23.0)
project(tiny-vulkan VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)
enable_testing()

add_library(tiny-vulkan tiny-vulkan.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(draw-triangle draw-triangle.cpp)