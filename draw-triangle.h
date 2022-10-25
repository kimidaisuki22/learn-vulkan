#pragma once
#include "GLFW/glfw3.h"
#include "tiny-vulkan.h"
#include <corecrt_startup.h>
#include <stdint.h>
#include <string>
#include <optional>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
// extra header for member functions.
#include <string_view>
#include <stdexcept>
#include <iostream>
#include <format>
#include <vector>
#include <span>
#include <unordered_set>

constexpr bool ENABLE_VALIDATION_LAYERS =
#ifdef NDEBUG
false;
#else 
true;
#endif

struct Queue_family_indices{
    std::optional <uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete()const{
        return graphics_family.has_value() && present_family.has_value();
    }
};

const std::vector<const char*> validation_layer_name_pointers{
    "VK_LAYER_KHRONOS_validation"
};

inline VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger
){
    auto static func =  (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");

    if(func){
        return func(instance,create_info,allocator,debug_messenger);
    }else{
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

inline void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator
){
    static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func){
        func(instance,debug_messenger,allocator);
    } else{
        throw std::runtime_error ("can't find extension");
    }
    
}

class HelloTriangleApp{
    public:
    HelloTriangleApp(uint32_t width=800,uint32_t height=600,std::string title = "Vulkan"):title_(std::move(title)),width_{width},height_{height}{}

    ~HelloTriangleApp(){
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
        setup_debug_messenger();

        create_surface();

        pick_physical_device();
        create_logical_device();
    }
    void main_loop(){
        while(!glfwWindowShouldClose(window_)){
            glfwPollEvents();
        }
    }
    void clean_up(){
        vkDestroyDevice(device_,nullptr);

        if constexpr (ENABLE_VALIDATION_LAYERS){
            DestroyDebugUtilsMessengerEXT(instance_,debug_messenger_,nullptr);
        }

        if(window_){
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        vkDestroySurfaceKHR(instance_,surface_,nullptr);
        if(instance_){
            vkDestroyInstance(instance_,nullptr);
            instance_ = nullptr;
        }
        glfwTerminate();
    }

    std::vector<const char*> get_required_extensions(){
        uint32_t glfw_extensions_count {};
        const char**  glfw_exntensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

        std::vector<const char*> extensions{glfw_exntensions,glfw_exntensions+glfw_extensions_count};

        if constexpr(ENABLE_VALIDATION_LAYERS){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
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


        auto extensions = get_required_extensions();
        create_info.enabledExtensionCount = extensions.size();
        create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if constexpr(ENABLE_VALIDATION_LAYERS){
            
            create_info.enabledLayerCount = validation_layer_name_pointers.size();
            create_info.ppEnabledLayerNames = validation_layer_name_pointers.data();

            populate_debug_messenger_create_info(debug_create_info);
            create_info.pNext = &debug_create_info;
        }else{
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        if(vkCreateInstance(&create_info,nullptr,&instance_) != VK_SUCCESS){
            throw std::runtime_error {"failed to create vulkan instance."};
        }


        {
            for(int i{};i<extensions.size();i++){
                std::cout << std::format("extension {}: {}\n",i,extensions[i]);
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
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info){
       create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT|
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

            create_info.messageType = 
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT|
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT|
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

            create_info.pfnUserCallback = debug_callback;
            create_info.pUserData = nullptr;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severiry,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data
    ){
        if(message_severiry <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT){
            return VK_FALSE;
        }
        std::cerr<< "validation layer: " << callback_data->pMessage <<"\n";
        auto vk_objects = std::span{callback_data->pObjects,callback_data->objectCount};

        return VK_FALSE;
    }

    void setup_debug_messenger(){
        if constexpr(ENABLE_VALIDATION_LAYERS){
            VkDebugUtilsMessengerCreateInfoEXT create_info{};
            populate_debug_messenger_create_info(create_info);            

            if(CreateDebugUtilsMessengerEXT(instance_,&create_info,nullptr,&debug_messenger_)!= VK_SUCCESS){
                throw std::runtime_error{"Failed to set up debug messenger"};
            }
        }
    }

    void pick_physical_device(){
        uint32_t device_count {};

        vkEnumeratePhysicalDevices(instance_,&device_count,nullptr);

        if(device_count == 0){
            throw std::runtime_error {"failed to find GPUs with Vulkan support!"};
        }
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_,&device_count, devices.data());

        for(auto device: devices){
            if(is_device_suitable(device)){
                physical_device_ = device;
                break;
            }
        }

        if(!physical_device_){
            throw std::runtime_error {"No suitable GPU found."};
        }
    }
    void create_logical_device(){
        Queue_family_indices indices = find_queue_families(physical_device_);
        std::vector<VkDeviceQueueCreateInfo> queue_infos;
        std::unordered_set<uint32_t> unique_queue_families {
            indices.graphics_family.value(),
            indices.present_family.value()
        };
        for(auto queue_family: unique_queue_families){
            VkDeviceQueueCreateInfo  queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family ;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority_;
            queue_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features{};
        VkDeviceCreateInfo create_info{};
        
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = queue_infos.data() ;
        create_info.queueCreateInfoCount = queue_infos.size();

        create_info.pEnabledFeatures = & device_features;

        if(vkCreateDevice(physical_device_,&create_info,nullptr,&device_) != VK_SUCCESS){
            throw  std::runtime_error {"failed to create logical device."};
        }

        vkGetDeviceQueue(device_, indices.graphics_family.value(), 0,&graphics_queue_); 
        vkGetDeviceQueue(device_, indices.present_family.value(), 0,&present_queue_); 

    }

    bool is_device_suitable(VkPhysicalDevice device){
        VkPhysicalDeviceProperties device_properties{};
        vkGetPhysicalDeviceProperties(device,&device_properties);

        VkPhysicalDeviceFeatures  device_feature{};
        vkGetPhysicalDeviceFeatures(device,&device_feature);

        return find_queue_families(device).is_complete();
    }

    Queue_family_indices find_queue_families(VkPhysicalDevice device){
        Queue_family_indices indices{};

        uint32_t queue_family_count {};
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queue_family_count,nullptr);
        
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queue_family_count, queue_families.data());

        for(int i{};const auto & queue_family : queue_families){
            if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                    indices.graphics_family = i;
            }
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface_,& present_support);
                if(present_support){
                    indices.present_family = i;
                }

            if(indices.is_complete()){
                break;
            }
            i++;
        }

        return indices;
    }

    void create_surface(){
        if(glfwCreateWindowSurface(instance_,window_,nullptr,&surface_)!=VK_SUCCESS){
            throw std::runtime_error{"failed to create window surface!"};
        }
    }

    private:
    std::string title_;
    GLFWwindow* window_{};
    VkInstance instance_{};
    VkSurfaceKHR surface_{};
    VkDebugUtilsMessengerEXT debug_messenger_;

    VkPhysicalDevice physical_device_ {VK_NULL_HANDLE};
    VkDevice device_{};
    VkQueue graphics_queue_{};
    VkQueue present_queue_{};

    uint32_t width_{};
    uint32_t height_{};
    const float queue_priority_ = 1.0f;
};