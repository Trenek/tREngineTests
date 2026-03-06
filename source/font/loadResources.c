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

static const char *const models[] = {
    "samples/fonts/b.ttf",
    "samples/fonts/c.ttf",
    "samples/fonts/d.ttf",
};
static const int32_t qModels = sizeof(models) / sizeof(const char *);
static int32_t currModel = 0;

static const char *const string[] = {
    "Objekt",
    "a",
    "b",
    "c",
    "tak"
};
static const int32_t qString = sizeof(string) / sizeof(const char *);
static int32_t currString = 0;

void moveNextFont(struct EngineCore *engine, enum state *state) {
    vkDeviceWaitIdle(engine->graphics.device);
    cleanupResourcesOrg(&engine->resource);
    engine->resource = (struct ResourceManager) {};

    state[1] = LOAD_RESOURCES;

    currModel += 1;
    currModel %= qModels;
}

void moveNextString(struct EngineCore *engine, enum state *state) {
    vkDeviceWaitIdle(engine->graphics.device);
    cleanupResourcesOrg(&engine->resource);
    engine->resource = (struct ResourceManager) {};

    state[1] = LOAD_RESOURCES;

    currString += 1;
    currString %= qString;
}
static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(textureManager, "PlaceHolder", loadTextures(&this->graphics, 1, (char *[]){ NULL }), unloadTextures);

    addResource(&this->resource, "textures", textureManager, cleanupResourceManager);
}

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, "Font", loadModel(models[currModel], &this->graphics), destroyActualModel);

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

    addResource(graphicPipelinesData, "Text", createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/text2dV.spv",
        .fragmentShader = "shaders/textF.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .texture = &colorTexture->descriptor,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .objectLayout = objectLayout->descriptorSetLayout,

        Vert(FontVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,

        .cameraLayout = cameraLayout->descriptorSetLayout
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, "graphicPipelines", graphicPipelinesData, cleanupResourceManager);
}

void addString(
    struct ResourceManager *entityData,
    struct ResourceManager *modelData,

    struct descriptorSetLayout *objectLayout,
    struct EngineCore *this,
    const char *buffer
) {
    addResource(entityData, buffer, createString((struct StringBuilder) {
        .instanceCount = 1,
        .string = string[currString],
        .modelData = findResource(modelData, "Font"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
        .center = 0
    }, &this->graphics), destroyEntity);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, "modelData");

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, "objectLayout"), "object");

    addString(entityData, modelData, objectLayout, this, "Object");

    addResource(&this->resource, "Entity", entityData, cleanupResourceManager);
}

void loadFontResources(struct EngineCore *engine, enum state *state) {
    addTextures(engine);
    addModelData(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;
}
