#pragma once
#include "tiny-vulkan.h"
#include <array>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> 
struct Vertex {
  glm::vec3 pos_;
  glm::vec3 normal_;
  glm::vec2 tex_coord_;

  bool operator==(const Vertex &other) const {
    return pos_ == other.pos_ && normal_ == other.normal_ &&
           tex_coord_ == other.tex_coord_;
  }

  static VkVertexInputBindingDescription get_binding_description() {
    VkVertexInputBindingDescription binding_description{};

    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
  }
  static std::array<VkVertexInputAttributeDescription, 3>
  get_attribute_descriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributes_description{};

    attributes_description[0].binding = 0;
    attributes_description[0].location = 0;
    attributes_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes_description[0].offset = offsetof(Vertex, pos_);

    attributes_description[1].binding = 0;
    attributes_description[1].location = 1;
    attributes_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes_description[1].offset = offsetof(Vertex, normal_);

    attributes_description[2].binding = 0;
    attributes_description[2].location = 2;
    attributes_description[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributes_description[2].offset = offsetof(Vertex, tex_coord_);

    return attributes_description;
  }
};
namespace std {
template <> struct hash<Vertex> {
  size_t operator()(Vertex const &vertex) const {
    return ((hash<glm::vec3>()(vertex.pos_) ^
             (hash<glm::vec3>()(vertex.normal_) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.tex_coord_) << 1);
  }
};
} // namespace std