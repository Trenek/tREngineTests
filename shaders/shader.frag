#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat uint fragTexIndex;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragVertex;
layout(location = 4) in flat uint fragMaterial;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

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

struct Materials {
    vec4 Ka;  /* Ambient */
    vec4 Kd;  /* Diffuse */
    vec4 Ks;  /* Specular */
    vec4 Ke;  /* Emission */
    vec4 Kt;  /* Transmittance */
    vec4 Tf;  /* Transmission filter */
    float Ns;     /* Shininess */
    float Ni;     /* Index of refraction */
    float d;      /* Disolve (alpha) */
    int illum;  /* Illumination model */

    /* Set for materials that don't come from the associated mtllib */
    int fallback;

    /* Texture map indices in fastObjMesh textures array */
    uint map_ka;
    uint map_kd;
    uint map_ks;
    uint map_ke;
    uint map_kt;
    uint map_ns;
    uint map_ni;
    uint map_d;
    uint map_bump;
    int pad0;
    int pad1;
};

layout(std140, set = 0, binding = 1) readonly buffer ObjectBuffer{
	Materials nr[];
} material;

layout(push_constant) uniform constants {
	int textureOffset;
} PushConstants;

void main() {
    Materials ma = material.nr[fragMaterial];
    int textureOffset = PushConstants.textureOffset;

    const vec3 ambientMap = texture(texSampler[ma.map_ka + textureOffset], fragTexCoord).rgb * ma.Ka.rgb;
    const vec3 diffuseMap = texture(texSampler[ma.map_kd + textureOffset], fragTexCoord).rgb * ma.Kd.rgb;
    const vec3 specularMap = texture(texSampler[ma.map_ks + textureOffset], fragTexCoord).rgb * ma.Ks.rgb;
    const vec3 emissionMap = texture(texSampler[ma.map_ke + textureOffset], fragTexCoord).rgb * ma.Ke.rgb;

    const vec3 lightDir = normalize(vec3(1.0f, 2.0f, 3.0f));
    const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
    const vec3 objectColor = vec3(1.0f, 0.5f, 0.31f);

    const vec3 ambient = ambientMap * lightColor;
    const vec3 diffuse = diffuseMap * max(dot(fragNormal, lightDir), 0.0) * lightColor;

    const vec3 viewDir = normalize(ubo.cameraPos.xyz - fragVertex);
    const vec3 reflectDir = reflect(-lightDir, fragNormal);  
    const float spec = pow(max(dot(viewDir, reflectDir), 0.0), ma.Ns);
    const vec3 specular = max(specularMap * spec * lightColor, 0);  

    const vec3 emission = emissionMap;

    const vec3 lightning = ambient + diffuse + specular + emission;

    outColor = vec4(lightning * objectColor, 1.0);
}
