#include "tiny-vulkan.h"
#include <iostream>
#include <format>

int main(){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    int width = 1920 ;
    int height = width * 16 / 9;
    GLFWwindow* window = glfwCreateWindow(width,height,"Vulkan Window",nullptr,nullptr);

    uint32_t extensionCount {};
    vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount,nullptr);

    std::cout << std::format ("{} extension supported.\n");

    glm::mat4 matrix;
    glm::vec4 vec;

    auto test = matrix * vec;
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
