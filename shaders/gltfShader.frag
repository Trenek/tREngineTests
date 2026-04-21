#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

struct Materials {
	vec4 base_color_factor;
	float metallic_factor;
	float roughness_factor;
    uint baseColorID;
    uint pad0;
};

layout(std140, set = 0, binding = 3) readonly buffer ObjectBuffer{
	Materials nr[];
} material;

layout(push_constant) uniform constants {
	int nodeID;
    int materialID;
} PushConstants;

void main() {
    Materials mat = material.nr[PushConstants.materialID];

    outColor = vec4(
        fragColor * 
        mat.base_color_factor.rgb *
        texture(texSampler[mat.baseColorID], fragTexCoords).rgb, 
    0.0);
}
