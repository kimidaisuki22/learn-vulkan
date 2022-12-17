#version 450

layout(set = 0, binding = 0) uniform Uniform_buffer_object{
    mat4 model_;
    mat4 view_;
    mat4 proj_;
} ubo;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_coord;

void main(){
    gl_Position = ubo.proj_ * ubo.view_ * ubo.model_ * vec4(in_position,0.0, 1.0);
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
}