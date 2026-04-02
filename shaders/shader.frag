#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat uint fragTexIndex;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragVertex;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

float minmax(float minn, float maxx, float val) {
    return max(minn, min(maxx, val));
}

void main() {
    const vec3 color = vec3(1, 1, 1);

    const vec3 lightColor = vec3(1.0, 0.0, 1.0);
    const vec3 direction = normalize(vec3(-1, -1, -1));

    const vec3 fdx = dFdx(fragVertex);
    const vec3 fdy = dFdy(fragVertex);
    const vec3 normal = normalize(cross(fdx, fdy));
    const float diff = minmax(0.1, 0.9, dot(normal, direction));

    outColor = vec4((diff * lightColor) * color, 1.0);
}
