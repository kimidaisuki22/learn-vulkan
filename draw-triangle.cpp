#include "tiny-vulkan.h"
#include "draw-triangle.h"
#include <iostream>
#include <format>

int main(){
    uint32_t extensionCount {};
    vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount,nullptr);

    std::cout << std::format ("{} extension supported.\n",extensionCount);

    glm::mat4 matrix;
    glm::vec4 vec;

    auto test = matrix * vec;
    
    
    HelloTriangleApp app;
    app.run();


    return 0;
}
