#version 450

layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec3 inNormal;
layout(location = 2) in  vec3 inColor;
layout(location = 3) in  vec2 inTexCoord;
layout(location = 4) in ivec4 inBoneIDs;
layout(location = 5) in  vec4 inWeights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat uint fragTexIndex;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragVertex;

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

layout(std140, set = 0, binding = 1) readonly buffer MeshBuffer{
	mat4 localModel[];
} mesh;

layout(std140, set = 0, binding = 2) readonly buffer AnimationBuffer{
    mat4 transformations[];
} anim;

layout(push_constant) uniform constants {
	int meshID;
} PushConstants;

vec4 applyAnimation(vec4 p) {
    return anim.transformations[PushConstants.meshID] * p;
}

void main() {
    vec4 position = applyAnimation(vec4(inPosition, 1.0));
    vec4 normal = applyAnimation(vec4(inNormal, 0.0));
    mat4 worldTransform = (
        instance.objects[gl_InstanceIndex].model *
        mesh.localModel[PushConstants.meshID]
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

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTexIndex = instance.objects[gl_InstanceIndex].index;
}
