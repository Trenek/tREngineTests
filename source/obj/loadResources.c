#include <cglm/cglm.h>

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

#include "objEnum.h"

#define MODEL(x) "samples/obj/data/"x".obj"

static const char *const models[] = {
    MODEL("alligator"),
    MODEL("armadillo"),
    MODEL("beast"),
    MODEL("beetle-alt"),
    MODEL("beetle"),
    MODEL("bimba"),
    MODEL("cheburashka"),
    MODEL("cow"),
    MODEL("fandisk"),
    MODEL("happy"),
    MODEL("homer"),
    MODEL("horse"), // nie ta pozycja
    MODEL("igea"),
    MODEL("lucy"), // nie ta pozycja
    MODEL("max-planck"),
    MODEL("nefertiti"), // nie ta pozycja
    MODEL("ogre"),
    MODEL("rocker-arm"),
    MODEL("spot"),
    MODEL("stanford-bunny"),
    MODEL("suzanne"),
    MODEL("teapot"),
    MODEL("woody"),
    MODEL("xyzrgb_dragon"),
};
static const int32_t qModels = sizeof(models) / sizeof(const char *);
static int32_t currModel = 0;

void moveNextObj(struct EngineCore *engine, enum state *state) {
    vkDeviceWaitIdle(engine->graphics.device);
    cleanupResourcesOrg(&engine->resource);
    engine->resource = (struct ResourceManager) {};

    state[1] = LOAD_RESOURCES;
}

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(textureManager, OBJ_TEXTURE_1, loadTextures(&this->graphics, 1, (char *[]){ NULL }), unloadTextures);

    addResource(&this->resource, OBJ_TEXTURES, textureManager, cleanupResourceManager);
}

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, OBJ_MODEL_1, loadModel(models[currModel], &this->graphics), destroyActualModel);

    addResource(&this->resource, OBJ_MODEL, modelData, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, OBJ_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, OBJ_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, OBJ_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, OBJ_OBJECT_LAYOUT_OBJECT, createDescriptorSetLayout(
        createObjectDescriptorSetLayout(this->graphics.device, 1, (VkDescriptorSetLayoutBinding []) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL
            },
        }), this->graphics.device), 
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, OBJ_OBJECT_LAYOUT_CAMERA, 
        createDescriptorSetLayout(createCameraDescriptorSetLayout(this->graphics.device), this->graphics.device), 
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, OBJ_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, OBJ_RENDER_PASS);

    struct Textures *colorTexture = findResource(findResource(&this->resource, OBJ_TEXTURES), OBJ_TEXTURE_1);
    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, OBJ_OBJECT_LAYOUT), OBJ_OBJECT_LAYOUT_OBJECT);
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, OBJ_OBJECT_LAYOUT), OBJ_OBJECT_LAYOUT_CAMERA);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, OBJ_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, OBJ_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, OBJ_GRAPHIC_PIPELINES_1, createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/vert.spv",
        .fragmentShader = "shaders/frag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .texture = &colorTexture->descriptor,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .objectLayout = objectLayout->descriptorSetLayout,

        Vert(ObjVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,

        .cameraLayout = cameraLayout->descriptorSetLayout
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, OBJ_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, OBJ_MODEL);

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, OBJ_OBJECT_LAYOUT), OBJ_OBJECT_LAYOUT_OBJECT);

    addResource(entityData, OBJ_ENTITIES_1, createModel((struct ModelBuilder) {
        .instanceCount = 1,
        .modelData = findResource(modelData, OBJ_MODEL_1),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, OBJ_ENTITIES, entityData, cleanupResourceManager);
}

void loadObjResources(struct EngineCore *engine, enum state *state) {
    addTextures(engine);
    addModelData(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;

    currModel += 1;
    currModel %= qModels;
}
