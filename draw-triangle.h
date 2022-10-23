#pragma once
#include "GLFW/glfw3.h"
#include "tiny-vulkan.h"
#include <stdint.h>
#include <string>
#include <vulkan/vulkan_core.h>

// extra header for member functions.
#include <string_view>
#include <stdexcept>
#include <iostream>
#include <format>
#include <vector>

constexpr bool ENABLE_VALIDATION_LAYERS =
#ifdef NDEBUG
false;
#else 
true;
#endif


std::vector<const char*> validation_layer_name_pointers{
    "VK_LAYER_KHRONOS_validation"
};

class HelloTriangleApp{
    public:
    HelloTriangleApp(uint32_t width=800,uint32_t height=600,std::string title = "Vulkan"):title_(std::move(title)),width_{width},height_{height}{}

    ~HelloTriangleApp(){
        clean_up();
    }
    void run(){
        init_window();
        init_vulkan();

        main_loop();

        clean_up();
    }

    private:
    void init_window(){
        glfwInit();

        // Do not create OpenGL context.
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);

        window_ = glfwCreateWindow(width_,height_,title_.c_str() ,nullptr,nullptr);
    }
    void init_vulkan(){
        create_instance();
    }
    void main_loop(){
        while(!glfwWindowShouldClose(window_)){
            glfwPollEvents();
        }
    }
    void clean_up(){
        if(window_){
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        if(instance_){
            vkDestroyInstance(instance_,nullptr);
            instance_ = nullptr;
        }
        glfwTerminate();
    }

    void create_instance(){
        if constexpr (ENABLE_VALIDATION_LAYERS){
            if(!check_validation_layer_support()){
                throw std::runtime_error("validation layers requested, but not available!");
            }
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "First Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Void Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;


        uint32_t glfw_extension_count {};
        const char** glfw_extensions {};

        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
        if constexpr(ENABLE_VALIDATION_LAYERS){
            
            create_info.enabledLayerCount = validation_layer_name_pointers.size();
            create_info.ppEnabledLayerNames = validation_layer_name_pointers.data();
        }else{
            create_info.enabledLayerCount = 0;
        }

        if(vkCreateInstance(&create_info,nullptr,&instance_) != VK_SUCCESS){
            throw std::runtime_error {"failed to create vulkan instance."};
        }


        {
            for(int i{};i<glfw_extension_count;i++){
                std::cout << std::format("extension {}: {}\n",i,glfw_extensions[i]);
            }
        }
 
    }

    void check_exntesions(){
        uint32_t exntension_count{};
        vkEnumerateInstanceExtensionProperties(nullptr,&exntension_count,nullptr);

        std::vector<VkExtensionProperties> properties(exntension_count);
        vkEnumerateInstanceExtensionProperties(nullptr,&exntension_count,properties.data());

        for(auto property: properties)
        {
            std::cout <<std::format("property name : {}, version: {}\n",property.extensionName,property.specVersion);
        }
    }

    bool check_validation_layer_support(){
        uint32_t layer_count{};
        vkEnumerateInstanceLayerProperties(&layer_count,nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count,available_layers.data());
        std::string_view validation_layer_name = validation_layer_name_pointers.front();

        for(auto & layer:available_layers){
            if(layer.layerName == validation_layer_name){
                return true;
            }
        }
        return false;
    }

    private:
    std::string title_;
    GLFWwindow* window_{};
    VkInstance instance_{};
    uint32_t width_{};
    uint32_t height_{};
};