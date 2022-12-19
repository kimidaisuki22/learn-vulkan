#version 450

layout(set = 0, binding = 0) uniform Uniform_buffer_object{
    mat4 model_;
    mat4 view_;
    mat4 proj_;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 frag_tex_coord;
layout(location = 2) out vec4 view_pos;

void main(){
    gl_Position = ubo.proj_ * ubo.view_ * ubo.model_ * vec4(in_position , 1.0);
    view_pos = ubo.proj_ * ubo.view_ * ubo.model_ * vec4(in_position , 1.0);
    normal = vec3(ubo.model_ * vec4(in_normal, 0));
    frag_tex_coord = in_tex_coord;
}