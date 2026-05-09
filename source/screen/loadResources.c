#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"
#include "texture.h"

#include "model.h"
#include "entity.h"
#include "rectangleBuilder.h"
#include "defaultInstance.h"
#include "defaultCamera.h"

#include "renderPassCore.h"

#include "descriptorSetLayoutObj.h"
#include "graphicsPipelineLayout.h"
#include "graphicsPipelineObj.h"

#include "rectangle.h"

#include "screenEnum.h"

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, SCREEN_MODEL_1, loadModel("model.scr", &this->graphics), destroyActualModel);

    addResource(&this->resource, SCREEN_MODEL, modelData, cleanupResourceManager);
}

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(
        textureManager, 
        SCREEN_TEXTURE_1,
        loadTextures(&this->graphics, 1, (struct TextureData []) {
            { .data = "samples/myTextures/random.png" }
        }), 
        unloadTextures
    );

    addResource(&this->resource, SCREEN_TEXTURES, textureManager, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, SCREEN_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, SCREEN_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, SCREEN_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, SCREEN_OBJECT_LAYOUT_OBJECT,
        defaultScreenDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, SCREEN_OBJECT_LAYOUT_CAMERA, 
        defaultCameraDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, SCREEN_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, SCREEN_OBJECT_LAYOUT), SCREEN_OBJECT_LAYOUT_CAMERA);

    addResource(graphicPipelinesData, SCREEN_GRAPHIC_PIPELINE_LAYOUT_1, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            cameraLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 1,
    }, &this->graphics), destroyPipelineLayoutObj);

    addResource(&this->resource, SCREEN_GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, SCREEN_RENDER_PASS);

    struct graphicsPipelineLayout *pipelineLayout = findResource(findResource(&this->resource, SCREEN_GRAPHIC_PIPELINE_LAYOUTS), SCREEN_GRAPHIC_PIPELINE_LAYOUT_1);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, SCREEN_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, SCREEN_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, SCREEN_GRAPHIC_PIPELINES_1, createGraphicsPipelineObj((struct GraphicsPipelineBuilder) {
        .pipelineLayout = pipelineLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/screenVert.spv",
        .fragmentShader = "shaders/synthwaveFrag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .vert = defaultRectVert(),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyPipelineObj);

    addResource(&this->resource, SCREEN_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, SCREEN_MODEL);

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, SCREEN_OBJECT_LAYOUT), SCREEN_OBJECT_LAYOUT_OBJECT);

    addResource(entityData, SCREEN_ENTITIES_1, createRec((struct RecBuilder) {
        .instanceCount = 1,
        .modelData = findResource(modelData, SCREEN_MODEL_1),
        .objectLayout = objectLayout->descriptorSetLayout,

        .instance = defaultInstance(),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, SCREEN_ENTITIES, entityData, cleanupResourceManager);
}

void loadScreenResources(struct EngineCore *engine, enum state *state) {
    addModelData(engine);
    addTextures(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;
}
