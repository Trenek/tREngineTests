#version 450

layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

layout(set = 2, binding = 0) readonly uniform UniformBufferObject {
    mat4 view;
    mat4 proj;

    vec4 lightDirection;
    vec4 lightColor;
    vec4 cameraPos;
} ubo;

struct ObjectData {
    uint index;
    mat4 model;
};

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} instance;

void main() {
    vec4 position = vec4(inPosition, 1.0);

    gl_Position = vec4((
        ubo.proj *
        instance.objects[gl_InstanceIndex].model *
        position
    ).xy, 0.0, 1.0);

    fragTexCoord = inTexCoord;
}
