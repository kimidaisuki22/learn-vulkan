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


add_executable(draw-triangle draw-triangle.cpp lib-impl.cpp)



add_subdirectory(env-setup)


target_link_libraries(tiny-vulkan PUBLIC glfw glm Vulkan::Vulkan)

target_link_libraries(draw-triangle PRIVATE tiny-vulkan)
if(true) # TODO:Just use fmt when std::format is not avaliable
target_link_libraries(draw-triangle PRIVATE fmt::fmt)
endif()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_program(glc glslc)
add_custom_target(shader_dir
COMMAND ${CMAKE_COMMAND} -E  make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/shaders
)
add_custom_target(shader
    COMMAND ${glc} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/vert.spv -c ${CMAKE_CURRENT_SOURCE_DIR}/vertex.vert
    COMMAND ${glc} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/frag.spv -c ${CMAKE_CURRENT_SOURCE_DIR}/fragment.frag
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vertex.vert ${CMAKE_CURRENT_SOURCE_DIR}/fragment.frag
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/shaders/vert.spv ${CMAKE_CURRENT_BINARY_DIR}/shaders/frag.spv
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling shaders"
    VERBATIM
)

add_dependencies(shader shader_dir)
add_dependencies(draw-triangle shader)