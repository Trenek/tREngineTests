#version 450

layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec3 inNormals;
layout(location = 2) in  vec2 inTexCoords;
layout(location = 3) in  vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoords;

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

layout(std140, set = 0, binding = 1) readonly buffer NodeBuffer1{
    mat4 transform[];
} node1;

layout(std140, set = 0, binding = 2) readonly buffer NodeBuffer2{
    mat4 animation[];
} node2;

layout(push_constant) uniform constants {
	int nodeID;
} PushConstants;

void main() {
    vec4 position = vec4(inPosition, 1.0);
    mat4 worldTransform = (
        instance.objects[gl_InstanceIndex].model *
        node2.animation[PushConstants.nodeID] *
        node1.transform[PushConstants.nodeID]
    );

    vec3 fragVertex = (
        worldTransform *
        position
    ).xyz;

    gl_Position = (
        ubo.proj *
        ubo.view *
        vec4(fragVertex, 1.0)
    );

    fragColor = inColor;
    fragTexCoords = inTexCoords;
}
