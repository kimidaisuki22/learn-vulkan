#include "tiny-vulkan.h"
#include "draw-triangle.h"
#include <iostream>
#include <format>

int main(int argc,const char ** argv){
    if(argc > 1){
        MODEL_PATH = argv[1];
    }
    if(argc > 2){
        TEXTURE_PATH = argv[2];
    }

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
