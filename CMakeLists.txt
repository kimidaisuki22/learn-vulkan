cmake_minimum_required(VERSION 3.23.0)
project(tiny-vulkan VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

find_package(Vulkan REQUIRED)
find_package(fmt REQUIRED)

add_library(tiny-vulkan tiny-vulkan.cpp)
target_include_directories(tiny-vulkan PUBLIC .)


set(IMGUI_DIR imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)
add_library(imgui  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
 
target_compile_definitions(imgui PUBLIC -DImTextureID=ImU64)
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

add_executable(draw-triangle draw-triangle.cpp lib-impl.cpp)



add_subdirectory(env-setup)


target_link_libraries(tiny-vulkan PUBLIC glfw glm Vulkan::Vulkan imgui)

target_link_libraries(draw-triangle PRIVATE tiny-vulkan)
if(true) # TODO:Just use fmt when std::format is not avaliable
target_link_libraries(draw-triangle PRIVATE fmt::fmt)
endif()

 


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
