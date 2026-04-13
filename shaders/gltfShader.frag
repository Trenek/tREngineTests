#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

struct Materials {
	vec4 base_color_factor;
	float metallic_factor;
	float roughness_factor;
};

layout(std140, set = 0, binding = 3) readonly buffer ObjectBuffer{
	Materials nr[];
} material;

void main() {
    outColor = vec4(
        fragColor * 
        material.nr[0].base_color_factor.rgb *
        texture(texSampler[0], fragTexCoords).rgb, 
    0.0);
}
