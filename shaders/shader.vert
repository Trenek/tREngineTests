#version 450

layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec3 inTexCoord;
layout(location = 2) in  vec3 inNormal;
layout(location = 3) in  vec2 inPara;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out flat uint fragTexIndex;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragVertex;

layout(set = 2, binding = 0) readonly uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

struct ObjectData {
    uint index;
    mat4 model;
};

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} instance;

layout(push_constant) uniform constants {
	int meshID;
} PushConstants;

void main() {
    vec4 position = vec4(inPosition, 1.0);
    vec4 normal = vec4(inNormal, 0.0);
    mat4 worldTransform = (
        instance.objects[gl_InstanceIndex].model
    );

    fragVertex = (
        worldTransform *
        position
    ).xyz;

    gl_Position = (
        ubo.proj *
        ubo.view *
        vec4(fragVertex, 1.0)
    );

    fragNormal = (
        worldTransform *
        normal
    ).xyz;

    fragTexCoord = inTexCoord.xy;
    fragTexIndex = instance.objects[gl_InstanceIndex].index;
}
