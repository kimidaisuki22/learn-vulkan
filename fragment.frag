#version 450

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) in vec4 view_pos;

layout(binding = 1) uniform sampler2D tex_sampler;

vec3 checker_board(vec2 uv){
    vec2 f = uv - floor(uv);
    return 
    ((f.x < 0.5) ^^ (f.y < 0.5f)) ? 
    vec3(1,1,1) :
    vec3(0,0,0);
}

void main(){
   vec3 color  = textureLod(tex_sampler, frag_tex_coord, log2(view_pos.w - 2)).rgb;
   
   // vec3 color = 0.6 * checker_board(frag_tex_coord * 10) +vec3(0.3,0.3,0.3);
    out_color = vec4(color , 1.0);
}