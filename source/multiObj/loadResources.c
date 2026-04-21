#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "objBuilder.h"
#include "instanceBuffer.h"

#include "renderPassCore.h"

#include "graphicsPipelineLayout.h"
#include "graphicsPipelineObj.h"

#include "obj.h"

#include "multiObjEnum.h"

#define MODEL_OTHER(x) "samples/otherOBJ/"x"/"x".obj"

static const char *const models[] = {
    MODEL_OTHER("papa"),
    MODEL_OTHER("korwin"),
    MODEL_OTHER("pilsudzki")
};
static const int32_t qModels = sizeof(models) / sizeof(const char *);

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    for (int32_t i = 0; i < qModels; i += 1) {
        addResource(modelData, i, loadModel(models[i], &this->graphics), destroyActualModel);
    }

    addResource(&this->resource, MULTI_OBJ_MODEL, modelData, cleanupResourceManager);
}

size_t countTextures(struct ResourceManager *modelManager, size_t *textureOffsets) {
    size_t result = 0;

    for (int32_t i = 0; i < qModels; i += 1) {
        textureOffsets[i] = result;
        result += ((struct Model*)findResource(modelManager, i))->qTexture;
    }

    return result;
}

void consolidateTextures(struct ResourceManager *modelManager, struct TextureData *textures) {
    int32_t k = 0;

    for (int32_t i = 0; i < qModels; i += 1) {
        struct Model *model = findResource(modelManager, i);
        for (uint32_t j = 0; j < model->qTexture; j += 1) {
            textures[k] = model->texture[j];
            k += 1;
        }
    }
}

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, MULTI_OBJ_MODEL);

    size_t *textureOffsets = malloc(qModels * sizeof(size_t));

    size_t qTextures = countTextures(modelData, textureOffsets);
    struct TextureData textures[qTextures];

    consolidateTextures(modelData, textures);

    addResource(
        textureManager, 
        MULTI_OBJ_TEXTURE_1, 
        loadTextures(&this->graphics, qTextures, textures), 
        unloadTextures
    );

    addResource(&this->resource, MULTI_OBJ_TEXTURE_OFFSETS, textureOffsets, free);
    addResource(&this->resource, MULTI_OBJ_TEXTURES, textureManager, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, MULTI_OBJ_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, MULTI_OBJ_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, MULTI_OBJ_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, MULTI_OBJ_OBJECT_LAYOUT_OBJECT, createDescriptorSetLayoutObj(
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
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL
            },
        }), this->graphics.device), 
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, MULTI_OBJ_OBJECT_LAYOUT_CAMERA, 
        createDescriptorSetLayoutObj(createCameraDescriptorSetLayout(this->graphics.device), this->graphics.device), 
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, MULTI_OBJ_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct Textures *colorTexture = findResource(findResource(&this->resource, MULTI_OBJ_TEXTURES), MULTI_OBJ_TEXTURE_1);
    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, MULTI_OBJ_OBJECT_LAYOUT), MULTI_OBJ_OBJECT_LAYOUT_OBJECT);
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, MULTI_OBJ_OBJECT_LAYOUT), MULTI_OBJ_OBJECT_LAYOUT_CAMERA);

    addResource(graphicPipelinesData, MULTI_OBJ_GRAPHIC_PIPELINE_LAYOUT_1, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
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
                .size = sizeof(struct ObjPushConstants),
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
            }
        }
    }, &this->graphics), destroyObjGraphicsPipelineLayout);

    addResource(&this->resource, MULTI_OBJ_GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, MULTI_OBJ_RENDER_PASS);

    struct graphicsPipelineLayout *pipelineLayout = findResource(findResource(&this->resource, MULTI_OBJ_GRAPHIC_PIPELINE_LAYOUTS), MULTI_OBJ_GRAPHIC_PIPELINE_LAYOUT_1);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, MULTI_OBJ_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, MULTI_OBJ_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, MULTI_OBJ_GRAPHIC_PIPELINES_1, createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .pipelineLayout = pipelineLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/vert.spv",
        .fragmentShader = "shaders/frag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        Vert(ObjVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, MULTI_OBJ_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, MULTI_OBJ_MODEL);

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, MULTI_OBJ_OBJECT_LAYOUT), MULTI_OBJ_OBJECT_LAYOUT_OBJECT);
    size_t *textureOffsets = findResource(&this->resource, MULTI_OBJ_TEXTURE_OFFSETS);

    for (int32_t i = 0; i < qModels; i += 1) {
        addResource(entityData, i, createObj((struct ObjBuilder) {
            .instanceCount = 1,
            .modelData = findResource(modelData, i),
            .objectLayout = objectLayout->descriptorSetLayout,

            INS(instance, instanceBuffer),

            .textureOffset = textureOffsets[i]
        }, &this->graphics), destroyEntity);
    }

    addResource(&this->resource, MULTI_OBJ_ENTITIES, entityData, cleanupResourceManager);
}

void loadMultiObjResources(struct EngineCore *engine, enum state *state) {
    addModelData(engine);
    addTextures(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;
}
