cmake_minimum_required(VERSION 3.23.0)
project(tiny-vulkan VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

find_package(Vulkan REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenMesh CONFIG REQUIRED)

find_package(Stb REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
add_library(tiny-vulkan tiny-vulkan.cpp)
target_include_directories(tiny-vulkan PUBLIC .)


# target_compile_definitions(imgui PUBLIC -DImTextureID=ImU64)
# target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

add_executable(draw-triangle draw-triangle.cpp objloader.cpp stb.cpp imLogger.cpp history.cpp OMesh.cpp stopwatch.cpp im_table.cpp)


target_link_libraries(tiny-vulkan PUBLIC glfw glm::glm Vulkan::Vulkan)

target_link_libraries(draw-triangle PRIVATE tiny-vulkan)
target_link_libraries(draw-triangle PRIVATE fmt::fmt)

target_link_libraries(draw-triangle PRIVATE spdlog::spdlog)

target_include_directories(draw-triangle PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(draw-triangle PRIVATE tinyobjloader::tinyobjloader)



target_sources(draw-triangle PRIVATE libs/ImGuiFileDialog/ImGuiFileDialog.cpp)
target_include_directories(draw-triangle PUBLIC libs)


target_include_directories(draw-triangle PRIVATE ${OPENMESH_INCLUDE_DIRS})
target_link_libraries(draw-triangle PRIVATE ${OPENMESH_LIBRARIES})
# On linux
target_link_libraries(draw-triangle PRIVATE OpenMeshCore OpenMeshTools)

target_link_libraries(draw-triangle PRIVATE ${OPENMESH_LIBRARIES})

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(draw-triangle PRIVATE unofficial::sqlite3::sqlite3)
# use sqlite-modern-cpp
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
target_include_directories(draw-triangle   PRIVATE ${SQLITE_MODERN_CPP_INCLUDE_DIRS})
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
target_link_libraries(draw-triangle PRIVATE implot::implot imgui::imgui)

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(draw-triangle PRIVATE CLI11::CLI11)

# find glslc to compile shaders
find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found")
endif()

# compile shaders
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/vert.spv
    COMMAND ${GLSLC} ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex.vert -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/vert.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex.vert
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/frag.spv
    COMMAND ${GLSLC} ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment.frag -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/frag.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment.frag
)
add_custom_target(shaders ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders/vert.spv ${CMAKE_CURRENT_BINARY_DIR}/shaders/frag.spv)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/font.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
