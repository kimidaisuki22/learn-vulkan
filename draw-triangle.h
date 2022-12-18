#pragma once
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "swap_chain.h"
#include "tiny-vulkan.h"
 
#include <cmath>
#include <compare>
#include <cstddef>
#include <cstdint>
 
#include <fstream>
#include <ios>
#include <limits>
#include <stdint.h>
#include <string>
#include <optional>
#include <type_traits>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
// extra header for member functions.
#include "sformat.h"
#include <string_view>
#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <vcruntime_string.h>
#include <vector>
#include <span>
#include <unordered_set>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <set>
#include <algorithm>
#include <chrono>

#include <stb/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

constexpr bool ENABLE_VALIDATION_LAYERS =
#if defined(NDEBUG) // || defined (__APPLE__)
false;
#else
true;
#endif
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/test_model.obj";
const std::string TEXTURE_PATH = "textures/test_texture.png";

struct Queue_family_indices{
    std::optional <uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete()const{
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct Vertex{
    glm::vec3 pos_;
    glm::vec3 color_;
    glm::vec2 tex_coord_;

    bool operator==(const Vertex& other) const {
        return pos_ == other.pos_ && color_ == other.color_ && tex_coord_ == other.tex_coord_;
    }

    static VkVertexInputBindingDescription get_binding_description(){
        VkVertexInputBindingDescription binding_description{};

        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }
    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions(){
        std::array<VkVertexInputAttributeDescription, 3> attributes_description{};

        attributes_description[0].binding = 0;
        attributes_description[0].location = 0;
        attributes_description[0].format =  VK_FORMAT_R32G32B32_SFLOAT;
        attributes_description[0].offset = offsetof(Vertex, pos_);

        attributes_description[1].binding = 0;
        attributes_description[1].location = 1;
        attributes_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes_description[1].offset = offsetof(Vertex, color_);

        attributes_description[2].binding = 0;
        attributes_description[2].location = 2;
        attributes_description[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributes_description[2].offset = offsetof(Vertex, tex_coord_);

        return attributes_description;
    }
};
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos_) ^
                   (hash<glm::vec3>()(vertex.color_) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.tex_coord_) << 1);
        }
    };
}

struct Uniform_buffer_object{
    alignas(16) glm::mat4 model_;
    alignas(16) glm::mat4 view_;
    alignas(16) glm::mat4 proj_;
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
    HelloTriangleApp(uint32_t width=800,uint32_t height=600,std::string title = "Vulkan"):title_(std::move(title)),width_{width},height_{height}{
        command_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
        render_finish_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
        in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);
    }

    ~HelloTriangleApp(){
    }
    void run(){
        init_window();
        init_vulkan();

        main_loop();

        cleanup();
    }

    private:
    void init_window(){
        if(glfwInit() !=GLFW_TRUE){
        throw            std::runtime_error{"failed to init glfw."};
        }

        // Do not create OpenGL context.
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);

        window_ = glfwCreateWindow(width_,height_,title_.c_str() ,nullptr,nullptr);
        if(!window_){
            throw std::runtime_error("failed to create window");
        }
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebuffer_resize_callback);
    }
    void init_vulkan(){
        create_instance();
        setup_debug_messenger();

        create_surface();

        pick_physical_device();
        create_logical_device();
        create_swap_chain();
        create_image_views();
        create_render_pass();
        create_descriptor_set_layout();
        create_graphics_pipeline();
        create_command_pool();
        create_color_resources();
        create_depth_resource();
        create_frame_buffers();
        create_texture_image();
        create_texture_image_view();
        create_texture_sampler();
        load_model();
        create_vertex_buffer();
        create_index_buffer();
        create_uniform_buffers();
        create_descriptor_pool();
        create_descriptor_sets();
        create_command_buffers();
        create_sync_objects();
    }
    void main_loop(){
        while(!glfwWindowShouldClose(window_)){
            showFPS(window_);
            glfwPollEvents();
            draw_frame();
        }
        vkDeviceWaitIdle(device_);
    }
    void cleanup(){
        cleanup_swap_chain();

        vkDestroySampler(device_, texture_sampler_, nullptr);
        vkDestroyImageView(device_, texture_image_view_, nullptr);
        vkDestroyImage(device_, texture_image_, nullptr);
        vkFreeMemory(device_, texture_image_memory_, nullptr);

        for(size_t i = 0 ; i < MAX_FRAMES_IN_FLIGHT; i++){
            vkDestroyBuffer(device_, uniform_buffers_[i], nullptr);
            vkFreeMemory(device_, uniform_buffers_memory_[i], nullptr);
        }

        vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
        vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);

        vkDestroyBuffer(device_, index_buffer_, nullptr);
        vkFreeMemory(device_, index_buffer_memory_, nullptr);
        vkDestroyBuffer(device_, vertex_buffer_, nullptr);
        vkFreeMemory(device_, vertex_buffer_memory_, nullptr);
        
        for(size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
            vkDestroyFence(device_, in_flight_fences_[i], nullptr);
            vkDestroySemaphore(device_, render_finish_semaphores_[i], nullptr);
            vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
        }

        vkDestroyCommandPool(device_, command_pool_, nullptr);

        vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
        vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
        vkDestroyRenderPass(device_, render_pass_, nullptr);
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
        if(glfw_exntensions == nullptr){
            throw std::runtime_error{"vulkan is not available on this machine."};
        }

        std::vector<const char*> extensions{glfw_exntensions,glfw_exntensions+glfw_extensions_count};

        if constexpr(ENABLE_VALIDATION_LAYERS){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        #ifdef __APPLE__
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

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
        #ifdef __APPLE__
        create_info.flags |=VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        if(auto result = vkCreateInstance(&create_info,nullptr,&instance_) ;result!= VK_SUCCESS){
            auto s= string_VkResult(result);
            std::cout << s<<"\n";
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
        // try install vulkan-validationlayers on linux if not found.
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
                msaa_samples_ = get_max_usable_sample_count();
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
        device_features.samplerAnisotropy = VK_TRUE;
        device_features.sampleRateShading = VK_TRUE;


        VkDeviceCreateInfo create_info{};

        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = queue_infos.data() ;
        create_info.queueCreateInfoCount = queue_infos.size();

        create_info.enabledExtensionCount = device_extensions_.size();
        create_info.ppEnabledExtensionNames = device_extensions_.data();

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

        const bool extensions_supported = check_extension_support(device);
        bool swap_chain_adequate {false};

        if(extensions_supported){
            auto swap_chain_support = query_swap_chain_details(device);
            swap_chain_adequate = !swap_chain_support.formats_.empty() && ! swap_chain_support.present_modes_.empty();
        }

        return find_queue_families(device).is_complete() && extensions_supported && swap_chain_adequate && device_feature.samplerAnisotropy;
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

    bool check_extension_support(VkPhysicalDevice device){
        uint32_t extension_count{};
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

        std::set<std::string> required_extensions{device_extensions_.begin(),device_extensions_.end()};

        for(const auto& extension: available_extensions){
            required_extensions.erase(extension.extensionName);
        }
        return required_extensions.empty();
    }
    Swap_chain_support_details query_swap_chain_details(VkPhysicalDevice device){
        Swap_chain_support_details details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities_);

        // Query formats.
        uint32_t format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);
        if(format_count){
            details.formats_.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, details.formats_.data());
        }

        // Query presentation modes.
        uint32_t present_mode_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count,nullptr);
        if(present_mode_count){
            details.present_modes_.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count,details.present_modes_.data());
        }



        return details;
    }
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats){
        for(auto available_format:available_formats){
            if(available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                return available_format;
            }
        }
        return available_formats.front();
    }

    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes){
        for(auto& available_present_mode:available_present_modes){
            if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR){ // Known as triple buffering.
                return available_present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be available.
    }
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities){
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
            return capabilities.currentExtent;
        }

        int width{};
        int height{};
        glfwGetFramebufferSize(window_, &width, &height);

        VkExtent2D actual_extent{
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
        };
        auto & min_extent = capabilities.minImageExtent;
        auto & max_extent = capabilities.maxImageExtent;
        actual_extent.height = std::clamp(actual_extent.height,min_extent.height,max_extent.height);
        actual_extent.width = std::clamp(actual_extent.width,min_extent.width,max_extent.width);

        return actual_extent;
    }

    void cleanup_swap_chain(){
        vkDestroyImageView(device_, color_image_view_, nullptr);
        vkDestroyImage(device_, color_image_, nullptr);
        vkFreeMemory(device_, color_image_memory_, nullptr);

        vkDestroyImageView(device_, depth_image_view_, nullptr);
        vkDestroyImage(device_, depth_image_, nullptr);
        vkFreeMemory(device_, depth_image_memory_, nullptr);

        for(size_t i = 0; i < swap_chain_frame_buffers_.size();i++){
            vkDestroyFramebuffer(device_, swap_chain_frame_buffers_[i], nullptr);
        }

        for(size_t i = 0; i < swap_chain_image_views_.size(); i++){
            vkDestroyImageView(device_, swap_chain_image_views_[i], nullptr);
        }
        vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
    }

    void create_swap_chain(){
        auto swap_chain_support = query_swap_chain_details(physical_device_);

        auto surface_format = choose_swap_surface_format(swap_chain_support.formats_);
        auto present_mode = choose_swap_present_mode(swap_chain_support.present_modes_);
        auto extent = choose_swap_extent(swap_chain_support.capabilities_);

        uint32_t image_count = swap_chain_support.capabilities_.minImageCount + 1; // Additional one to avoid wait on the driver to complete internal operations before we can accquire another image to render to.

        VkSwapchainCreateInfoKHR create_info{};
        // fill create info for create swap chain.
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surface_;

        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;

        create_info.imageArrayLayers = 1; // always 1 unless developing stereo 3D application.
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        Queue_family_indices indices = find_queue_families(physical_device_);
        uint32_t queue_family_indices[] = {
            indices.graphics_family.value(),
            indices.present_family.value()
        };

        if(indices.graphics_family != indices.present_family){
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        }
        else{
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0 ;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = swap_chain_support.capabilities_.currentTransform;

        //see: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        if(vkCreateSwapchainKHR(device_, &create_info, nullptr, &swap_chain_)!=VK_SUCCESS){
            throw std::runtime_error{"failed to create swap chain."};
        }

        vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count,nullptr);
       swap_chain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, swap_chain_images_.data());

       swap_chain_image_format_ = surface_format.format;
       swap_chain_extent_ = extent;
    }

    void recreate_swap_chain(){
        int width{};
        int height{};
        glfwGetWindowSize(window_, &width, &height);
        while(width == 0 || height == 0){
            glfwGetWindowSize(window_, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device_);
        
        cleanup_swap_chain();

        create_swap_chain();
        create_image_views();
        create_color_resources();
        create_depth_resource();
        create_frame_buffers();
    }

    void create_image_views(){
        swap_chain_image_views_.resize(swap_chain_images_.size());

        for(size_t i = 0 ; i < swap_chain_image_views_.size();i++){
          swap_chain_image_views_[i] = create_image_view(swap_chain_images_[i], swap_chain_image_format_,VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }
    void create_graphics_pipeline(){
        auto vert_shader_code = read_file("shaders/vert.spv");
        auto frag_shader_code = read_file("shaders/frag.spv");

        auto vert_shader_module = create_shader_module(vert_shader_code);
        auto frag_shader_module = create_shader_module(frag_shader_code);


        VkPipelineShaderStageCreateInfo vert_shader_create_info{};
        vert_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_create_info.stage= VK_SHADER_STAGE_VERTEX_BIT;

        vert_shader_create_info.module = vert_shader_module;
        vert_shader_create_info.pName = "main"; // the function to invoke. aka entrypoint.

        VkPipelineShaderStageCreateInfo frag_shader_create_info{};
        frag_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        frag_shader_create_info.module =frag_shader_module;
        frag_shader_create_info.pName = "main";

        VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_create_info,frag_shader_create_info};

        // Fixed Pipeline.

        // Dyncamic part:
        std::vector<VkDynamicState> dynamic_states {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = dynamic_states.size();
        dynamic_state.pDynamicStates = dynamic_states.data();


        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        
        auto binding_description = Vertex::get_binding_description();
        auto attribute_descriptions = Vertex::get_attribute_descriptions();

        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = &binding_description;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        // viewport.
        VkViewport view_port {};
        view_port.x = 0;
        view_port.y =0;
        view_port.width =static_cast<float>(swap_chain_extent_.width);
        view_port.height =static_cast<float>(swap_chain_extent_.height);
        view_port.minDepth = 0;
        view_port.maxDepth = 1;

        VkRect2D scissor{};
        scissor.offset = {0,0};
        scissor.extent = swap_chain_extent_;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &view_port;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        // depth and stencil
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
        
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.0f;

        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {};
        depth_stencil.back = {};

        // rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE; // if never pass rasterizer stage.
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;

        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        rasterizer.depthBiasEnable =VK_FALSE;

        // Multisampling.
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.rasterizationSamples = msaa_samples_;
        multisampling.minSampleShading = 0.8f; // closer to 1 is smoother.
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        //Depth and stencil testing.
        // not needed now.

        // Color blending.
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if constexpr(true)// no blend?
        {
            color_blend_attachment.blendEnable = VK_FALSE;
            color_blend_attachment.srcColorBlendFactor =VK_BLEND_FACTOR_ONE;
            color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;

            color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }else{
            color_blend_attachment.blendEnable = VK_TRUE;
            color_blend_attachment.srcColorBlendFactor =VK_BLEND_FACTOR_SRC_ALPHA;
            color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;

            color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp =VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        
        // Pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &pipeline_layout_)!=VK_SUCCESS){
            throw std::runtime_error{"failed to create pipeline layout"};
        }

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;

        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pDepthStencilState = &depth_stencil;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state;

        pipeline_info.layout = pipeline_layout_;

        pipeline_info.renderPass = render_pass_;
        pipeline_info.subpass = 0;

        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;

        if(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != VK_SUCCESS){
            throw std::runtime_error{"failed to create graphics pipeline."};
        }
        
        vkDestroyShaderModule(device_, vert_shader_module, nullptr);
        vkDestroyShaderModule(device_, frag_shader_module, nullptr);
    }

    void create_render_pass(){
        VkAttachmentDescription color_attachment{};
        color_attachment.format = swap_chain_image_format_;
        color_attachment.samples = msaa_samples_;

        // color and depth.
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Subpassese and attachment references
        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = find_depth_format();
        depth_attachment.samples = msaa_samples_;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription color_attachment_resolve{};
        color_attachment_resolve.format = swap_chain_image_format_;
        color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_resolve_ref{};
        color_attachment_resolve_ref.attachment = 2;
        color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;
        subpass.pResolveAttachments = &color_attachment_resolve_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;

        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3> attachments ={
            color_attachment, 
            depth_attachment,
            color_attachment_resolve,
        };

        // Render pass
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if(vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_) != VK_SUCCESS){
            throw std::runtime_error{"failed to create render pass."};
        }
    }

    void create_frame_buffers(){
        swap_chain_frame_buffers_.resize(swap_chain_image_views_.size());

        for(size_t i=0 ;i < swap_chain_image_views_.size();i++){
            std::array<VkImageView,3> attachments ={
                color_image_view_,
                depth_image_view_,
                swap_chain_image_views_[i],
            };

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = render_pass_;
            framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_info.pAttachments = attachments.data();
            framebuffer_info.width = swap_chain_extent_.width;
            framebuffer_info.height = swap_chain_extent_.height;
            framebuffer_info.layers = 1;

            if(vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &swap_chain_frame_buffers_[i])!= VK_SUCCESS){
                throw std::runtime_error{"failed to create framebuffer."};
            }
        }
    }
    void create_command_pool(){
        Queue_family_indices queue_family_indices = find_queue_families(physical_device_);

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

        if(vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_)!=VK_SUCCESS){
            throw std::runtime_error{"failed to create command pool."};
        }
    }
    void create_command_buffers(){
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool_;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if(vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data())!=VK_SUCCESS){
            throw std::runtime_error{"failed to allocate command buffer."};
        }
    }

    void record_command_buffer(VkCommandBuffer command_buffer,uint32_t image_index){
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType =VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(command_buffer, &begin_info)!=VK_SUCCESS){
            throw std::runtime_error{"failed to begin record commmand buffer."};
        }


        // Starting a render pass
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_pass_;
        render_pass_info.framebuffer = swap_chain_frame_buffers_[image_index];

        render_pass_info.renderArea.offset = {0,0};
        render_pass_info.renderArea.extent = swap_chain_extent_;

        std::array<VkClearValue,2> clear_values{};
        clear_values[0].color = {{0.0f,0.0f,0.0f,1.0f}};
        clear_values[1].depthStencil = {1.0f,0};
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        // Basic drawing commands
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

        VkViewport viewport{};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(swap_chain_extent_.width);
        viewport.height = static_cast<float>(swap_chain_extent_.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0,0};
        scissor.extent = swap_chain_extent_;
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
        
        VkBuffer vertex_buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &descriptor_sets_[current_frame_], 0, nullptr);

        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);

        // Finishing up
        vkCmdEndRenderPass(command_buffer);
        if(vkEndCommandBuffer(command_buffer)!=VK_SUCCESS){
            throw  std::runtime_error{"failed to record command buffer."};
        }
    }

    void draw_frame(){
        vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

        uint32_t image_index{};
        VkResult result =  vkAcquireNextImageKHR(device_, swap_chain_, UINT64_MAX, image_available_semaphores_[current_frame_], VK_NULL_HANDLE, &image_index);

        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            recreate_swap_chain();
            return;
        }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error{"failed to acquire swap chain image."};
        }

        vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

        // call before submitting next frame.
        update_uniform_buffer(current_frame_);

        // record commands.
        vkResetCommandBuffer(command_buffers_[current_frame_], 0);
        record_command_buffer(command_buffers_[current_frame_], image_index);
        
        // submit commands.
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores [] = {image_available_semaphores_[current_frame_]};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffers_[current_frame_];

        VkSemaphore signal_semaphores[] = {render_finish_semaphores_[current_frame_]};
        submit_info.signalSemaphoreCount =1;
        submit_info.pSignalSemaphores = signal_semaphores;

        if(vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[current_frame_])!=VK_SUCCESS){
            throw std::runtime_error{"failed to submit draw command buffer."};
        }

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swap_chains[] = {swap_chain_};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swap_chains;
        present_info.pImageIndices = &image_index;
        present_info.pResults = nullptr;

        result = vkQueuePresentKHR(graphics_queue_, &present_info);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized_){
            framebuffer_resized_ = false;
            recreate_swap_chain();
        }
        else if(result != VK_SUCCESS){
            throw std::runtime_error{"failed to present swap chain image."};
        }

        current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    void create_sync_objects(){
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for(size_t i =0;i<MAX_FRAMES_IN_FLIGHT;i++){
            if(
                vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finish_semaphores_[i]) != VK_SUCCESS ||
                vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i])
            ){
                throw  std::runtime_error{"failed to create semaphores."};
            }
        }
    }

    void create_vertex_buffer(){
        VkDeviceSize size =  sizeof(vertices_[0]) * vertices_.size();
        
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create_buffer(size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            staging_buffer,staging_buffer_memory);

         
        void * data{};
        vkMapMemory(device_, staging_buffer_memory, 0,size, 0, &data);
        memcpy(data, vertices_.data(), static_cast<size_t>(size));
        vkUnmapMemory(device_, staging_buffer_memory);

        create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_);

        copy_buffer(staging_buffer, vertex_buffer_, size);
        
        vkDestroyBuffer(device_, staging_buffer, nullptr);
        vkFreeMemory(device_, staging_buffer_memory, nullptr);
    }
    void create_index_buffer(){
        VkDeviceSize size =  sizeof(indices_[0]) *indices_.size();
        
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create_buffer(size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            staging_buffer,staging_buffer_memory);

         
        void * data{};
        vkMapMemory(device_, staging_buffer_memory, 0,size, 0, &data);
        memcpy(data, indices_.data(), static_cast<size_t>(size));
        vkUnmapMemory(device_, staging_buffer_memory);

        create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT| VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_, index_buffer_memory_);

        copy_buffer(staging_buffer, index_buffer_, size);
        
        vkDestroyBuffer(device_, staging_buffer, nullptr);
        vkFreeMemory(device_, staging_buffer_memory, nullptr);
    }
    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

        for(uint32_t i = 0; i <mem_properties.memoryTypeCount; i++){
            if(type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties){
                return i;
            }
        }
        throw std::runtime_error{"failed to find suitable memory type."};
    }
    
    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory){
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;

        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(vkCreateBuffer(device_, &buffer_info, nullptr, &buffer)!=VK_SUCCESS){
            throw std::runtime_error{"failed to create buffer"};
        }
        VkMemoryRequirements mem_requirements{};
        vkGetBufferMemoryRequirements(device_, buffer,&mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

        if(vkAllocateMemory(device_, &alloc_info, nullptr, &buffer_memory) !=VK_SUCCESS)   {
            throw std::runtime_error{"failed to allocate vertex buffer for merory"};
        }
        vkBindBufferMemory(device_, buffer, buffer_memory, 0);
    }
    void copy_buffer(VkBuffer src_buffer,VkBuffer dst_buffer, VkDeviceSize size){
         VkCommandBuffer command_buffer = begin_single_time_commands();

        VkBufferCopy copy_region{};
        copy_region.dstOffset = 0;
        copy_region.srcOffset = 0;
        copy_region.size = size;
        
        vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
        
        end_single_time_commands(command_buffer);
    }

    void create_descriptor_set_layout(){
        VkDescriptorSetLayoutBinding ubo_layout_binging{};
        ubo_layout_binging.binding = 0;
        ubo_layout_binging.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binging.descriptorCount = 1;

        ubo_layout_binging.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binging.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding sampler_layout_binging{};
        sampler_layout_binging.binding = 1;
        sampler_layout_binging.descriptorCount = 1;
        sampler_layout_binging.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_layout_binging.pImmutableSamplers = nullptr;
        sampler_layout_binging.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
            ubo_layout_binging,
            sampler_layout_binging,
        };
        
        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();

        if(vkCreateDescriptorSetLayout(device_, &layout_info, nullptr, &descriptor_set_layout_) != VK_SUCCESS){
            throw std::runtime_error{"failed to create descriptor set layout."};
        }
    }

    void create_uniform_buffers(){
        VkDeviceSize buffer_size = sizeof(Uniform_buffer_object);

        uniform_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        uniform_buffers_memory_.resize(MAX_FRAMES_IN_FLIGHT);
        uniform_buffers_mapped_.resize(MAX_FRAMES_IN_FLIGHT);

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT ;i++){
            create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , uniform_buffers_[i], uniform_buffers_memory_[i]);

            vkMapMemory(device_, uniform_buffers_memory_[i], 0, buffer_size, 0, &uniform_buffers_mapped_[i]);
        }
    }

    void update_uniform_buffer(uint32_t current_image){
        static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        // https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer#page_Updating-uniform-data
        Uniform_buffer_object ubo{};
        ubo.model_ = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),glm::vec3(0,0,1));
        ubo.view_ = glm::lookAt(glm::vec3(2,2,2), glm::vec3(0,0,0), glm::vec3(0,0,1));
        const auto FOV = glm::radians(45.0f);
        ubo.proj_ = glm::perspective(FOV, static_cast<float>(swap_chain_extent_.width)/static_cast<float>(swap_chain_extent_.height), 0.1f, 10.0f);
        ubo.proj_[1][1] *= -1;
        memcpy(uniform_buffers_mapped_[current_image], &ubo, sizeof ubo);
    }

    void create_descriptor_pool(){
        std::array<VkDescriptorPoolSize,2> pool_sizes{};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType =  VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if(vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS){
            throw std::runtime_error{"failed to create descriptor pool."};
        }
    }

    void create_descriptor_sets(){
        std::vector<VkDescriptorSetLayout> layouts (MAX_FRAMES_IN_FLIGHT,descriptor_set_layout_);

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool_;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        alloc_info.pSetLayouts = layouts.data();

        descriptor_sets_.resize(MAX_FRAMES_IN_FLIGHT);
        if(vkAllocateDescriptorSets(device_, &alloc_info, descriptor_sets_.data()) != VK_SUCCESS){
            throw std::runtime_error{"failed to allocate descriptor sets."};
        }

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = uniform_buffers_[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(Uniform_buffer_object);
            
            VkDescriptorImageInfo image_info{};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.sampler = texture_sampler_;
            image_info.imageView = texture_image_view_;

            std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
            descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[0].dstSet = descriptor_sets_[i];
            descriptor_writes[0].dstBinding = 0;
            descriptor_writes[0].dstArrayElement = 0;

            descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[0].descriptorCount = 1;

            descriptor_writes[0].pBufferInfo = &buffer_info;
            descriptor_writes[0].pImageInfo = nullptr;
            descriptor_writes[0].pTexelBufferView = nullptr;

            descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[1].dstSet = descriptor_sets_[i];
            descriptor_writes[1].dstBinding = 1;
            descriptor_writes[1].dstArrayElement = 0;
            descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_writes[1].descriptorCount = 1;
            descriptor_writes[1].pImageInfo = &image_info;
            vkUpdateDescriptorSets(device_, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
        }
    }

    void create_texture_image(){
        int tex_width{};
        int tex_height{};
        int tex_channels{};

        stbi_uc * pixels =stbi_load(TEXTURE_PATH.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if(!pixels){
            throw std::runtime_error{"failed to load texture image."};
        }
        
        VkDeviceSize image_size = tex_height * tex_width * 4;
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        
        mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width,tex_height)))) + 1;

        create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

        {
            void *data;
            vkMapMemory(device_, staging_buffer_memory, 0, image_size, 0, &data);
            memcpy(data, pixels, image_size);
            vkUnmapMemory(device_, staging_buffer_memory);
        }
        stbi_image_free(pixels);
        pixels = nullptr;        

        create_image(static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_width), mip_levels_, VK_SAMPLE_COUNT_1_BIT,VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |VK_IMAGE_USAGE_SAMPLED_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image_, texture_image_memory_);

        transition_image_layout(texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels_);
        copy_buffer_to_image(staging_buffer, texture_image_, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));

        // transition_image_layout(texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels_);


        vkDestroyBuffer(device_, staging_buffer, nullptr);
        vkFreeMemory(device_, staging_buffer_memory, nullptr);
        
        generate_mipmaps(texture_image_, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, mip_levels_);
    }
    
    void create_image(uint32_t width, uint32_t height,uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,VkImage& image, VkDeviceMemory& image_memory){
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = static_cast<uint32_t>(width);
        image_info.extent.height = static_cast<uint32_t>(height);
        image_info.extent.depth = 1;
        image_info.mipLevels = mip_levels;
        image_info.arrayLayers = 1;

        image_info.format = format;
        image_info.tiling = tiling;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = usage;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.samples = num_samples;
        image_info.flags = 0;

        if(vkCreateImage(device_, &image_info, nullptr, &image) != VK_SUCCESS){
            throw std::runtime_error{"failed to create image."};
        }

        VkMemoryRequirements mem_requirements{};
        vkGetImageMemoryRequirements(device_, image, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

        if(vkAllocateMemory(device_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS){
            throw std::runtime_error{"failed to allocate iamge memory."};
        }

        vkBindImageMemory(device_, image, image_memory, 0);
    }
    
    VkCommandBuffer begin_single_time_commands(){
       VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool_;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo being_info{};
        being_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        being_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        vkBeginCommandBuffer(command_buffer, &being_info);

        return command_buffer;
    }

    void end_single_time_commands(VkCommandBuffer command_buffer){
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue_);
        vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer);
    }

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels){
        VkCommandBuffer command_buffer = begin_single_time_commands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mip_levels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
            barrier.srcAccessMask = 0; 
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; 

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }else{
            throw std::invalid_argument{"unsupported layout transition."};
        }

        if(new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL){
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if(has_stencil_component(format)){
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }else 
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        

        vkCmdPipelineBarrier(command_buffer,
         source_stage, destination_stage,
         0, 0, nullptr, 0, nullptr,
         1, &barrier);

        end_single_time_commands(command_buffer);
    }

    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width,uint32_t height){
        auto command_buffer = begin_single_time_commands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        end_single_time_commands(command_buffer);
    }

    VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels){
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.subresourceRange.aspectMask = aspect_flags;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = mip_levels;
        view_info.subresourceRange.baseArrayLayer =0;
        view_info.subresourceRange.layerCount = 1;

        VkImageView image_view;
        if(vkCreateImageView(device_, &view_info, nullptr, &image_view) != VK_SUCCESS){
            throw std::runtime_error{"failed to create texture iamge view."};
        }
        return image_view;
    } 
    void create_texture_image_view(){
        texture_image_view_ = create_image_view(texture_image_, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_ASPECT_COLOR_BIT,mip_levels_);
    }

    void create_texture_sampler(){
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        sampler_info.magFilter = VK_FILTER_NEAREST;
        sampler_info.magFilter = VK_FILTER_NEAREST;

        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        sampler_info.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physical_device_, &properties);
        sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;

        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = static_cast<float>(mip_levels_);

        if(vkCreateSampler(device_, &sampler_info, nullptr, &texture_sampler_) != VK_SUCCESS){
            throw std::runtime_error{"failed to create texture sampler."};
        }
    }

    void create_depth_resource(){
        auto depth_format = find_depth_format();

        create_image(swap_chain_extent_.width, swap_chain_extent_.height, 1, msaa_samples_,  depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image_, depth_image_memory_);

        depth_image_view_ = create_image_view(depth_image_, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        transition_image_layout(depth_image_, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, 1);
    }

    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
        for(VkFormat format :candidates){
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device_, format, &props);

            if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features){
                return format;
            }else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features){
                return format;
            }
        }

        throw std::runtime_error{"failed to find supported format."};
    }
    VkFormat find_depth_format(){
        return find_supported_format(
            {
                VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,
            }, 
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
    bool has_stencil_component(VkFormat format){
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void load_model(){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn,err;

        if(!tinyobj::LoadObj(&attrib, &shapes,&materials, &warn,&err,MODEL_PATH.c_str())){
            throw std::runtime_error{warn + err};
        }

        std::unordered_map<Vertex, uint32_t> unique_vertices{};

        for(const auto& shape: shapes){
            for(const auto& index: shape.mesh.indices){
                Vertex vertex{};

                vertex.pos_ = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.tex_coord_ = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                }; 

                vertex.color_ = {1.0f,1.0f,1.0f};

                if(unique_vertices.count(vertex) == 0){
                    unique_vertices[vertex] = static_cast<uint32_t>(unique_vertices.size());
                    vertices_.push_back(vertex);
                }
                indices_.push_back(unique_vertices[vertex]);
            }
        }
    }

    void generate_mipmaps(VkImage image, VkFormat image_format, uint32_t tex_width, uint32_t tex_height, uint32_t mip_levels){
        // Check if image suport linear blitting.
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties(physical_device_, image_format, &format_properties);

        if(!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)){
            throw std::runtime_error{"texture iamge format does not suport linear blitting."};
        }


        VkCommandBuffer command_buffer = begin_single_time_commands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mip_width = tex_width;
        int32_t mip_height = tex_height;
 
        for(uint32_t i = 1; i < mip_levels; i++){
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 
            0, nullptr, 0, nullptr, 
            1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mip_width, mip_height, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;

            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {std::max(mip_width/2 ,1), std::max(mip_height/2 ,1), 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(command_buffer,
             image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
             image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
             1, &blit, 
             VK_FILTER_LINEAR);

             barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
             barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
             barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

             vkCmdPipelineBarrier(command_buffer, 
             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
             0, nullptr, 
             0, nullptr, 
             1, &barrier);

             if(mip_width > 1){
                mip_width /= 2;
             }
             if(mip_height > 1){
                mip_height /= 2;
             }
        }

        barrier.subresourceRange.baseMipLevel = mip_levels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
        0, nullptr, 
        0, nullptr, 
        1, &barrier);

        end_single_time_commands(command_buffer);
    }

    VkSampleCountFlagBits get_max_usable_sample_count(){
        VkPhysicalDeviceProperties physical_device_properties{};
        vkGetPhysicalDeviceProperties(physical_device_, &physical_device_properties);

        VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void create_color_resources(){
        VkFormat color_format = swap_chain_image_format_;

        create_image(swap_chain_extent_.width, swap_chain_extent_.height, 1, msaa_samples_, color_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color_image_, color_image_memory_);
        color_image_view_ = create_image_view(color_image_, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }



    static std::vector<char> read_file(std::string_view filename){
        std::ifstream file(static_cast<std::string>(filename),std::ios::ate|std::ios::binary);

        if(!file.is_open()){
            throw std::runtime_error{std::format("can't open file {}",filename)};
        }
        size_t total_size  = file.tellg();
        std::vector<char> buffer(total_size);

        file.seekg(0);
        file.read(buffer.data(), total_size);
        file.close();

        return buffer;
    }
    static void showFPS(GLFWwindow *pWindow)
    {
        // Measure speed
        double currentTime = glfwGetTime();
        static double lastTime{};
        static uint64_t nbFrames{};
        double delta = currentTime - lastTime;
        nbFrames++;
        if ( delta >= 1.0 ){ // If last cout was more than 1 sec ago
          

            double fps = double(nbFrames) / delta;

          
            auto str = std::format(" [{} FPS]",fps);

            glfwSetWindowTitle(pWindow, str.c_str());

            nbFrames = 0;
            lastTime = currentTime;
        }
    }
    static void framebuffer_resize_callback(GLFWwindow* window,int width,int height){
        auto app = reinterpret_cast<HelloTriangleApp*>(glfwGetWindowUserPointer(window));
        app->framebuffer_resized_ = true;
    }
    VkShaderModule create_shader_module(const std::vector<char>& code){
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule module{};
        if(vkCreateShaderModule(device_, &create_info, nullptr, &module)!=VK_SUCCESS){
            throw std::runtime_error{"Create shader module failed."};
        }

        return module;
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

    const std::vector<const char*> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    uint32_t width_{};
    uint32_t height_{};
    uint32_t current_frame_{};
    const float queue_priority_ = 1.0f;
    VkSwapchainKHR swap_chain_;
    std::vector<VkImage> swap_chain_images_;
    std::vector<VkImageView> swap_chain_image_views_;
    VkFormat swap_chain_image_format_;
    VkExtent2D swap_chain_extent_;
    VkRenderPass render_pass_;
    VkDescriptorSetLayout descriptor_set_layout_;
    VkPipelineLayout pipeline_layout_;
    VkPipeline graphics_pipeline_;

    std::vector<VkFramebuffer> swap_chain_frame_buffers_;
    VkCommandPool command_pool_;
    std::vector<VkCommandBuffer> command_buffers_;

    // synchronization
    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finish_semaphores_;
    std::vector<VkFence> in_flight_fences_;

    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
    VkBuffer vertex_buffer_;
    VkDeviceMemory vertex_buffer_memory_;
    VkBuffer index_buffer_;
    VkDeviceMemory index_buffer_memory_;

    std::vector<VkBuffer> uniform_buffers_;
    std::vector<VkDeviceMemory> uniform_buffers_memory_;
    std::vector<void*> uniform_buffers_mapped_{};

    VkDescriptorPool descriptor_pool_;
    std::vector<VkDescriptorSet> descriptor_sets_;

    uint32_t mip_levels_;
    VkImage texture_image_;
    VkDeviceMemory texture_image_memory_;
    VkImageView texture_image_view_;
    VkSampler texture_sampler_;

    VkImage depth_image_;
    VkDeviceMemory depth_image_memory_;
    VkImageView depth_image_view_;

    VkSampleCountFlagBits msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;
    VkImage color_image_;
    VkDeviceMemory color_image_memory_;
    VkImageView color_image_view_;


    bool framebuffer_resized_ = false;
};