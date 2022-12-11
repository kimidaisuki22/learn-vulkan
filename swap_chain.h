#pragma once
#include "tiny-vulkan.h"
#include <vector>
#include <vulkan/vulkan_core.h>

class Swap_chain{

};

struct Swap_chain_support_details{
    VkSurfaceCapabilitiesKHR capabilities_;
    std::vector<VkSurfaceFormatKHR>formats_;
    std::vector<VkPresentModeKHR> present_modes_;
};
