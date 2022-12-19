#include "tiny-vulkan.h"
#include "draw-triangle.h"
#include <iostream>
#include <format>

int main(int argc,const char ** argv){
    uint32_t extensionCount {};
    vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount,nullptr);

    std::cout << std::format ("{} extension supported.\n",extensionCount);
    
    HelloTriangleApp app;
    app.run();


    return 0;
}
