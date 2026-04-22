#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "gltfBuilder.h"
#include "instanceBuffer.h"

#include "renderPassCore.h"

#include "graphicsPipelineLayout.h"
#include "graphicsPipelineObj.h"

#include "gltf.h"

#include "gltfEnum.h"

#define MODEL(x) "samples/glTF/Models/"x"/glTF/"x".gltf"
#define MODEL_EMB(x) "samples/glTF/Models/"x"/glTF-Embedded/"x".gltf"
#define MODEL_BIN(x) "samples/glTF/Models/"x"/glTF-Binary/"x".glb"

#define MY_MODEL(x) "samples/myglTF/"x".glb"

static const char *const models[] = {
    MY_MODEL("player"),
    MY_MODEL("czlowiek"),
    MODEL("SimpleSkin"),
    MODEL("AnimatedTriangle"),
    MODEL("SimpleMorph"),
    MODEL_BIN("WaterBottle"),
    MODEL_EMB("SimpleTexture"),
    MODEL("SimpleMaterial"),
    MODEL("SimpleMeshes"),
    MODEL("SimpleSparseAccessor"),
    MODEL("TriangleWithoutIndices"),
    MODEL("Triangle"),
};
static const int32_t qModels = sizeof(models) / sizeof(const char *);
static int32_t currModel = 0;

void moveNextGltf(struct EngineCore *engine, enum state *state) {
    vkDeviceWaitIdle(engine->graphics.device);
    cleanupResourcesOrg(&engine->resource);
    engine->resource = (struct ResourceManager) {};

    state[1] = LOAD_RESOURCES;
}

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, GLTF_MODEL_1, loadModel(models[currModel], &this->graphics), destroyActualModel);

    addResource(&this->resource, GLTF_MODEL, modelData, cleanupResourceManager);
}

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));
    struct Model *modelData = findResource(findResource(&this->resource, GLTF_MODEL), GLTF_MODEL_1);

    addResource(
        textureManager, 
        GLTF_TEXTURE_1, 
        loadTextures(&this->graphics, modelData->qTexture, modelData->texture), 
        unloadTextures
    );

    addResource(&this->resource, GLTF_TEXTURES, textureManager, cleanupResourceManager);
}


static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, GLTF_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, GLTF_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, GLTF_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, GLTF_OBJECT_LAYOUT_OBJECT,
        defaultGltfDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, GLTF_OBJECT_LAYOUT_CAMERA, 
        defaultCameraDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, GLTF_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct Textures *colorTexture = findResource(findResource(&this->resource, GLTF_TEXTURES), GLTF_TEXTURE_1);
    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, GLTF_OBJECT_LAYOUT), GLTF_OBJECT_LAYOUT_OBJECT);
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, GLTF_OBJECT_LAYOUT), GLTF_OBJECT_LAYOUT_CAMERA);

    addResource(graphicPipelinesData, GLTF_GRAPHIC_PIPELINE_LAYOUT_1, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            objectLayout->descriptorSetLayout,
            colorTexture->descriptor.descriptorSetLayout,
            cameraLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 3,

        .pushConstantRangeCount = 1,
        .pPushConstantRanges = (VkPushConstantRange []) {
            {
                .offset = 0,
                .size = sizeof(struct GltfPushConstants),
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            }
        }
    }, &this->graphics), destroyObjGraphicsPipelineLayout);

    addResource(&this->resource, GLTF_GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, GLTF_RENDER_PASS);

    struct graphicsPipelineLayout *pipelineLayout = findResource(findResource(&this->resource, GLTF_GRAPHIC_PIPELINE_LAYOUTS), GLTF_GRAPHIC_PIPELINE_LAYOUT_1);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, GLTF_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, GLTF_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, GLTF_GRAPHIC_PIPELINES_1, createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .pipelineLayout = pipelineLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/gltfVert.spv",
        .fragmentShader = "shaders/gltfFrag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        Vert(GltfVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, GLTF_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, GLTF_MODEL);

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, GLTF_OBJECT_LAYOUT), GLTF_OBJECT_LAYOUT_OBJECT);

    addResource(entityData, GLTF_ENTITIES_1, createGltf((struct GltfBuilder) {
        .instanceCount = 1,
        .modelData = findResource(modelData, GLTF_MODEL_1),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, GLTF_ENTITIES, entityData, cleanupResourceManager);
}

void loadGltfResources(struct EngineCore *engine, enum state *state) {
    addModelData(engine);
    addTextures(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;

    currModel += 1;
    currModel %= qModels;
}
