#include <cglm.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "modelBuilder.h"
#include "stringBuilder.h"
#include "instanceBuffer.h"

#include "renderPassCore.h"

#include "graphicsPipelineObj.h"

#include "Vertex.h"

static const char *models[] = {
    "samples/glTF/Models/Box/glTF-Binary/Box.glb",
    "samples/glTF/Models/RiggedSimple/glTF-Binary/RiggedSimple.glb",
};
static const int32_t qModels = sizeof(models) / sizeof(const char *);
static int32_t currModel = 0;

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(textureManager, "PlaceHolder", loadTextures(&this->graphics, 1, (const char *[]){
        "icon/iconka.png",
    }), unloadTextures);

    addResource(&this->resource, "textures", textureManager, cleanupResourceManager);
}

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, "Box", loadModel(models[currModel], &this->graphics), destroyActualModel);

    addResource(&this->resource, "modelData", modelData, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, "Clean", createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, "Stay", createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, "RenderPassCoreData", renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, "object", createDescriptorSetLayout(
        createObjectDescriptorSetLayout(this->graphics.device, 2, (VkDescriptorSetLayoutBinding []) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL
            },
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL
            }
        }), this->graphics.device), 
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, "camera", 
        createDescriptorSetLayout(createCameraDescriptorSetLayout(this->graphics.device), this->graphics.device), 
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, "objectLayout", objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, "RenderPassCoreData");

    struct Textures *colorTexture = findResource(findResource(&this->resource, "textures"), "PlaceHolder");
    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, "objectLayout"), "object");
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, "objectLayout"), "camera");

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, "GLTF-Pipe", createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/vert.spv",
        .fragmentShader = "shaders/frag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .texture = &colorTexture->descriptor,
        .objectLayout = objectLayout->descriptorSetLayout,

        Vert(AnimVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,

        .cameraLayout = cameraLayout->descriptorSetLayout
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, "graphicPipelines", graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, "modelData");

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, "objectLayout"), "object");

    addResource(entityData, "Object", createModel((struct ModelBuilder) {
        .instanceCount = 1,
        .modelData = findResource(modelData, "Box"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, "Entity", entityData, cleanupResourceManager);
}

void loadResources(struct EngineCore *engine, enum state *state) {
    addTextures(engine);
    addModelData(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelines(engine);
    addEntities(engine);

    *state = LOAD_TEST;

    currModel += 1;
    currModel %= qModels;
}
