#version 450
struct Light {
    vec3 position;
    vec3 color;
};


layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) in vec4 view_pos;
layout(location = 3) in vec3 camera_pos;


layout(binding = 1) uniform sampler2D tex_sampler;
layout(std140, binding = 2) uniform Lights {
    Light lightss[4];
};

vec3 checker_board(vec2 uv){
    vec2 f = uv - floor(uv);
    return
    ((f.x < 0.5) ^^ (f.y < 0.5f)) ?
    vec3(1,1,1) :
    vec3(0.5,0.5,0.5);
}
// 0 - 360
vec3 hue_to_rgb(float hue){

    float saturation = 0.4f;
    float lightness = 0.5f;

    float chroma = (1.0f - abs(2.0f * lightness - 1.0f)) * saturation;
    float x = chroma * (1.0f - abs(mod(hue / 60.0f, 2.0f) - 1.0f));
    float m = lightness - chroma / 2.0f;

    vec3 color2 = vec3(0.0f);

    if (hue < 60.0f) {
        color2 = vec3(chroma, x, 0.0f);
    } else if (hue < 120.0f) {
        color2 = vec3(x, chroma, 0.0f);
    } else if (hue < 180.0f) {
        color2 = vec3(0.0f, chroma, x);
    } else if (hue < 240.0f) {
        color2 = vec3(0.0f, x, chroma);
    } else if (hue < 300.0f) {
        color2 = vec3(x, 0.0f, chroma);
    } else {
        color2 = vec3(chroma, 0.0f, x);
    }
    return color2 + m;
}

// 0 - 1  blue to red
vec3 heat_map(float heat){
    return vec3(heat,0,1-heat);
}

void main(){
   // vec3 color  = textureLod(tex_sampler, frag_tex_coord, log2(view_pos.w - 2)).rgb;

     vec3 color = 0.6 * checker_board(view_pos.xy * 10) +vec3(0.3,0.3,0.3);

    vec3 toLight =  camera_pos - view_pos.xyz;
    vec3 lightDir = normalize(toLight);

    float density = max(dot(normal, lightDir), 0);

    // out_color = vec4(heat_map(density) *0.7f ,1.0f);
    out_color = vec4(vec3(density),1.0f);

     return;

    if(density<0.1){
        out_color = vec4(0.7f,0.7f,0,1); // - vec4(color, 0);
        return;
    }
    if(density>0.99){
        out_color = vec4(0,0,0.9,1); // - vec4(color, 0);
        return;
    }

    out_color =vec4(color * density,1.0f);

     return;
    // vec3 color = vec3(1,1,1);


    vec3 diffuse = vec3(0.0);
    Light lights[6] = {
    {vec3(-10.0, 0.0, 100.0), vec3(1.0, 1.0, 1.0)},
    {vec3(0.0, 10.0, 0.0), vec3(1.0, 0.0, 0.0)},
    {vec3(0.0, -10.0, 0.0), vec3(0.0, 1.0, 0.0)},
    {vec3(0.0, 0.0, -100.0), vec3(0.0, 0.0, 1.0)},

    {vec3( 10.0,0.0, 0.0), vec3(1.0, 0.0, 0.0)},
    {vec3( -10.0,0.0, 0.0), vec3(0.0, 1.0, 0.0)},
    };


    for (int i = 0; i < lights.length() ; i++) {
        // Compute the direction from the current fragment to the light
        vec3 toLight = lights[i].position - view_pos.xyz;
        vec3 lightDir = normalize(toLight);

        // Compute the diffuse color from this light
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 lightColor = lights[i].color;
        vec3 diffuseContrib = diffuseFactor * lightColor;
        diffuse += diffuseContrib;
    }
    color = color * diffuse;
    out_color = vec4(color , 1.0);


}