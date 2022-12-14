#pragma once
#include "GLFW/glfw3.h"
#include "glm/ext/vector_float2.hpp"
#include "swap_chain.h"
#include "tiny-vulkan.h"
// #include <corecrt_startup.h>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <limits>
#include <stdint.h>
#include <string>
#include <optional>
#include <glm/glm.hpp>
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

constexpr bool ENABLE_VALIDATION_LAYERS =
#if defined(NDEBUG) // || defined (__APPLE__)
false;
#else
true;
#endif
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Queue_family_indices{
    std::optional <uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete()const{
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct Vertex{
    glm::vec2 pos_;
    glm::vec3 color_;

    static VkVertexInputBindingDescription get_binding_description(){
        VkVertexInputBindingDescription binding_description{};

        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }
    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions(){
        std::array<VkVertexInputAttributeDescription, 2> attributes_description{};

        attributes_description[0].binding = 0;
        attributes_description[0].location = 0;
        attributes_description[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes_description[0].offset = offsetof(Vertex, pos_);

        attributes_description[1].binding = 0;
        attributes_description[1].location = 1;
        attributes_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes_description[1].offset = offsetof(Vertex, color_);

        return attributes_description;
    }
};
const std::vector<Vertex> vertices{
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};
const std::vector<uint32_t> indices {
    0,1,2,2,3,0
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
        create_graphics_pipeline();
        create_frame_buffers();
        create_command_pool();
        create_vertex_buffer();
        create_index_buffer();
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

        return find_queue_families(device).is_complete() && extensions_supported && swap_chain_adequate;
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
        create_swap_chain();
        create_image_views();
        create_frame_buffers();
    }

    void create_image_views(){
        swap_chain_image_views_.resize(swap_chain_images_.size());

        for(size_t i = 0 ; i < swap_chain_image_views_.size();i++){
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image=swap_chain_images_[i];

            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = swap_chain_image_format_;

            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0 ;
            create_info.subresourceRange.levelCount = 1 ;
            create_info.subresourceRange.baseArrayLayer = 0 ;
            create_info.subresourceRange.layerCount = 1 ;

            if(vkCreateImageView(device_, &create_info, nullptr, swap_chain_image_views_.data()+ i) != VK_SUCCESS){
                throw std::runtime_error{"failed to create image view."};
            }
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

        // rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE; // if never pass rasterizer stage.
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;

        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        rasterizer.depthBiasEnable =VK_FALSE;

        // Multisampling.
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.f;
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
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
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
        pipeline_info.pDepthStencilState = nullptr;
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
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

        // color and depth.
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Subpassese and attachment references
        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;

        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Render pass
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
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
            VkImageView attachments[] ={
                swap_chain_image_views_[i]
            };

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = render_pass_;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
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

        VkClearValue clear_color = {{{0.0f,0.0f,0.0f,1.0f,}}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

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

        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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
        VkDeviceSize size =  sizeof(vertices[0]) * vertices.size();
        
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create_buffer(size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            staging_buffer,staging_buffer_memory);

         
        void * data{};
        vkMapMemory(device_, staging_buffer_memory, 0,size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(size));
        vkUnmapMemory(device_, staging_buffer_memory);

        create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_);

        copy_buffer(staging_buffer, vertex_buffer_, size);
        
        vkDestroyBuffer(device_, staging_buffer, nullptr);
        vkFreeMemory(device_, staging_buffer_memory, nullptr);
    }
    void create_index_buffer(){
        VkDeviceSize size =  sizeof(indices[0]) *indices.size();
        
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create_buffer(size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            staging_buffer,staging_buffer_memory);

         
        void * data{};
        vkMapMemory(device_, staging_buffer_memory, 0,size, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(size));
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

        VkBufferCopy copy_region{};
        copy_region.dstOffset = 0;
        copy_region.srcOffset = 0;
        copy_region.size = size;
        
        vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue_);
        vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer);
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
    VkPipelineLayout pipeline_layout_;
    VkPipeline graphics_pipeline_;

    std::vector<VkFramebuffer> swap_chain_frame_buffers_;
    VkCommandPool command_pool_;
    std::vector<VkCommandBuffer> command_buffers_;

    // synchronization
    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finish_semaphores_;
    std::vector<VkFence> in_flight_fences_;

    VkBuffer vertex_buffer_;
    VkDeviceMemory vertex_buffer_memory_;
    VkBuffer index_buffer_;
    VkDeviceMemory index_buffer_memory_;

    bool framebuffer_resized_ = false;
};