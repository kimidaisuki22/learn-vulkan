cmake_minimum_required(VERSION 3.23.0)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TEST OFF)
set(GLFW_BUILD_DOCS OFF)

include(FetchContent)
 
FetchContent_Declare(
    GLM
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
)

FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 7482de6071d21db77a7236155da44c172a7f6c9e
)

FetchContent_MakeAvailable(GLFW GLM)