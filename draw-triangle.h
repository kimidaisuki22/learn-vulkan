#pragma once
#include "GLFW/glfw3.h"
#include "tiny-vulkan.h"
#include <stdint.h>
#include <string>
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
        glfwTerminate();
    }

    private:
    std::string title_;
    GLFWwindow* window_{};
    uint32_t width_{};
    uint32_t height_{};
};