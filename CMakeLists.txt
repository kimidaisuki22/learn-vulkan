cmake_minimum_required(VERSION 3.23.0)
project(tiny-vulkan VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

include_directories("C:/VulkanSDK/1.3.224.1/Include")
link_directories("C:/VulkanSDK/1.3.224.1/Lib")

add_library(tiny-vulkan tiny-vulkan.cpp)


add_executable(draw-triangle draw-triangle.cpp)

add_subdirectory(env-setup)



target_link_libraries(tiny-vulkan PUBLIC glfw glm vulkan-1)
target_link_libraries(draw-triangle PRIVATE tiny-vulkan)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)