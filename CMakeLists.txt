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
 
add_subdirectory(env-setup)

include_directories("C:/VulkanSDK/1.3.224.1/Include")
link_directories("C:/VulkanSDK/1.3.224.1/Lib")


target_link_libraries(tiny-vulkan PUBLIC glfw glm)
target_link_libraries(draw-triangle PRIVATE tiny-vulkan)