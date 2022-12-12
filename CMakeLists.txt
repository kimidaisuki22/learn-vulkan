cmake_minimum_required(VERSION 3.23.0)
project(tiny-vulkan VERSION 0.0.1)

link_directories(/usr/local/lib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

find_package(Vulkan REQUIRED)
find_package(fmt REQUIRED)

add_library(tiny-vulkan tiny-vulkan.cpp)


add_executable(draw-triangle draw-triangle.cpp)

target_include_directories(draw-triangle PUBLIC /usr/local/include)

add_subdirectory(env-setup)


target_link_libraries(tiny-vulkan PUBLIC glfw glm Vulkan::Vulkan)

target_link_libraries(draw-triangle PRIVATE tiny-vulkan)
if(true) # TODO:Just use fmt when std::format is not avaliable
target_link_libraries(draw-triangle PRIVATE fmt)
endif()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
