#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "fontBuilder.h"
#include "instanceBuffer.h"

#include "renderPassCore.h"

#include "graphicsPipelineObj.h"
#include "graphicsPipelineLayout.h"

#include "font.h"

#include "fontEnum.h"

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

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, FONT_MODEL_1, loadModel(models[currModel], &this->graphics), destroyActualModel);

    addResource(&this->resource, FONT_MODEL, modelData, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, FONT_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, FONT_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, FONT_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, FONT_OBJECT_LAYOUT_OBJECT, createDescriptorSetLayoutObj(
        createDescriptorSetLayout(this->graphics.device, 2, (VkDescriptorSetLayoutBinding []) {
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
    addResource(objectLayoutData, FONT_OBJECT_LAYOUT_CAMERA, createDescriptorSetLayoutObj(
        createDescriptorSetLayout(this->graphics.device, 1, (VkDescriptorSetLayoutBinding []) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL
            }
        }), this->graphics.device), 
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, FONT_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, FONT_OBJECT_LAYOUT), FONT_OBJECT_LAYOUT_OBJECT);
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, FONT_OBJECT_LAYOUT), FONT_OBJECT_LAYOUT_CAMERA);

    addResource(graphicPipelinesData, FONT_GRAPHIC_PIPELINE_LAYOUT_1, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            objectLayout->descriptorSetLayout,
            cameraLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 2,

        .pushConstantRangeCount = 1,
        .pPushConstantRanges = (VkPushConstantRange []) {
            {
                .offset = 0,
                .size = sizeof(struct FontPushConstants),
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
            }
        }
    }, &this->graphics), destroyObjGraphicsPipelineLayout);

    addResource(&this->resource, FONT_GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, FONT_RENDER_PASS);

    struct graphicsPipelineLayout *pipelineLayout = findResource(findResource(&this->resource, FONT_GRAPHIC_PIPELINE_LAYOUTS), FONT_GRAPHIC_PIPELINE_LAYOUT_1);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, FONT_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, FONT_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, FONT_GRAPHIC_PIPELINES_1, createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .pipelineLayout = pipelineLayout->pipelineLayout,

        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/text2dV.spv",
        .fragmentShader = "shaders/textF.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        Vert(FontVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, FONT_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

void addString(
    struct ResourceManager *entityData,
    struct ResourceManager *modelData,

    struct descriptorSetLayout *objectLayout,
    struct EngineCore *this,
    size_t id
) {
    addResource(entityData, id, createFont((struct FontBuilder) {
        .instanceCount = 1,
        .string = string[currString],
        .modelData = findResource(modelData, FONT_MODEL_1),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
        .center = 0
    }, &this->graphics), destroyEntity);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, FONT_MODEL);

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, FONT_OBJECT_LAYOUT), FONT_OBJECT_LAYOUT_OBJECT);

    addString(entityData, modelData, objectLayout, this, FONT_ENTITIES_1);

    addResource(&this->resource, FONT_ENTITIES, entityData, cleanupResourceManager);
}

void loadFontResources(struct EngineCore *engine, enum state *state) {
    addModelData(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;
}
