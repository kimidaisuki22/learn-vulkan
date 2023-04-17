#include "tiny-vulkan.h"
#include "draw-triangle.h"
#include <CLI/App.hpp>
#include <iostream>
#include <fmt/format.h>
#include <CLI/CLI.hpp> 

int main(int argc,const char ** argv){
    CLI::App fix{"fix a broken mesh","fixer"};
    fix.add_option("--model",MODEL_PATH);
    fix.add_option("--texture",TEXTURE_PATH);
    // CLI11_PARSE(fix);

    CLI11_PARSE(fix ,argc,argv);
    // CLI11_PARSE(fix);
    uint32_t extensionCount {};
    vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount,nullptr);

    std::cout << fmt::format ("{} extension supported.\n",extensionCount);

    HelloTriangleApp app;
    app.run();


    return 0;
}
