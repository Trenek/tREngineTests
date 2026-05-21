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
#include "descriptorObj.h"
#include "imageObj.h"

#include "rectangle.h"

#include "compTextEnum.h"

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, COMP_TEXT_MODEL_1, loadModel("model.scr", &this->graphics), destroyActualModel);

    addResource(&this->resource, COMP_TEXT_MODEL, modelData, cleanupResourceManager);
}

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(textureManager, COMP_TEXT_TEXTURE_1,
        loadUintTextures(&this->graphics, 1, (struct TextureData []) {
            { .data = "samples/myTextures/random.png" }
        }), 
        unloadTextures
    );

    addResource(&this->resource, COMP_TEXT_TEXTURES, textureManager, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, COMP_TEXT_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, COMP_TEXT_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, COMP_TEXT_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, COMP_TEXT_OBJECT_LAYOUT_OBJECT,
        defaultScreenDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, COMP_TEXT_OBJECT_LAYOUT_CAMERA, 
        defaultCameraDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, COMP_TEXT_OBJECT_LAYOUT_COMPUTE, createDescriptorSetLayoutObj(1, (VkDescriptorSetLayoutBinding[]) {
        [0] = {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImmutableSamplers = NULL,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        }
    }, this->graphics.device), destroyDescriptorSetLayout);

    addResource(objectLayoutData, COMP_TEXT_OBJECT_LAYOUT_TEXTURE, createDescriptorSetLayoutObj(1, (VkDescriptorSetLayoutBinding[]) {
        [0] = {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImmutableSamplers = NULL,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        }
    }, this->graphics.device), destroyDescriptorSetLayout);

    addResource(&this->resource, COMP_TEXT_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *objectLayoutManager = findResource(&this->resource, COMP_TEXT_OBJECT_LAYOUT);

    struct DescriptorSetLayout *computeLayout = findResource(objectLayoutManager, COMP_TEXT_OBJECT_LAYOUT_COMPUTE);
    // struct Textures *texture = findResource(findResource(&this->resource, COMP_TEXT_TEXTURES), COMP_TEXT_TEXTURE_1);

    struct DescriptorSetLayout *textureLayout = findResource(findResource(&this->resource, COMP_TEXT_OBJECT_LAYOUT), COMP_TEXT_OBJECT_LAYOUT_TEXTURE);

    addResource(graphicPipelinesData, COMP_TEXT_PIPELINE_LAYOUT_GRAPHICS, createPipelineLayout((struct PipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []) {
            textureLayout->descriptorSetLayout
        },
        .qDescriptorSetLayout = 1,
        .debugName = "Uklad Potoku Graficznego",
    }, &this->graphics), destroyPipelineLayoutObj);

    addResource(graphicPipelinesData, COMP_TEXT_COMPUTE_PIPELINE_LAYOUT, createPipelineLayout((struct PipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []) {
            computeLayout->descriptorSetLayout
        },
        .qDescriptorSetLayout = 1,
        .debugName = "Uklad Potoku Obliczeniowego",
    }, &this->graphics), destroyPipelineLayoutObj);

    addResource(&this->resource, COMP_TEXT_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, COMP_TEXT_RENDER_PASS);
    struct ResourceManager *pipelineLayoutData = findResource(&this->resource, COMP_TEXT_PIPELINE_LAYOUTS);

    struct PipelineLayout *graphicsPipelineLayout = findResource(pipelineLayoutData, COMP_TEXT_PIPELINE_LAYOUT_GRAPHICS);
    struct PipelineLayout *computePipelineLayout = findResource(pipelineLayoutData, COMP_TEXT_COMPUTE_PIPELINE_LAYOUT);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, COMP_TEXT_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, COMP_TEXT_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, COMP_TEXT_GRAPHIC_PIPELINES_1, createGraphicsPipelineObj((struct GraphicsPipelineBuilder) {
        .pipelineLayout = graphicsPipelineLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/screenVert.spv",
        .fragmentShader = "shaders/textureFrag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .vert = defaultRectVert(),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
        .debugName = "Potok Graficzny",
    }, &this->graphics), destroyPipelineObj);

    addResource(graphicPipelinesData, COMP_TEXT_COMPUTE_PIPELINE, createComputePipelineObj((struct ComputePipelineBuilder) {
        .computeShader = "shaders/textureComp.spv",
        .pipelineLayout = computePipelineLayout->pipelineLayout,
        .debugName = "Potok Obliczeniowy",
    }, &this->graphics), destroyPipelineObj);

    addResource(&this->resource, COMP_TEXT_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, COMP_TEXT_MODEL);

    struct DescriptorSetLayout *objectLayout = findResource(findResource(&this->resource, COMP_TEXT_OBJECT_LAYOUT), COMP_TEXT_OBJECT_LAYOUT_OBJECT);

    addResource(entityData, COMP_TEXT_ENTITIES_1, createRec((struct RecBuilder) {
        .instanceCount = 1,
        .modelData = findResource(modelData, COMP_TEXT_MODEL_1),
        .objectLayout = objectLayout->descriptorSetLayout,

        .instance = defaultInstance(),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, COMP_TEXT_ENTITIES, entityData, cleanupResourceManager);
}

static void createTextures(struct EngineCore *this) {
    struct DescriptorSetLayout *textureLayoutComp = findResource(findResource(&this->resource, COMP_TEXT_OBJECT_LAYOUT), COMP_TEXT_OBJECT_LAYOUT_COMPUTE);
    struct DescriptorSetLayout *textureLayoutFrag = findResource(findResource(&this->resource, COMP_TEXT_OBJECT_LAYOUT), COMP_TEXT_OBJECT_LAYOUT_TEXTURE);

    addResource(&this->resource, COMP_TEXT_WRITE_TEXTURE, createImageObj((struct ImageBuilder) {
        .extent = this->graphics.swapChain.extent
    }, &this->graphics), destroyImageObj);
    struct ImageObj *image = findResource(&this->resource, COMP_TEXT_WRITE_TEXTURE);

    addResource(&this->resource, COMP_TEXT_DESCRIPTOR_COMP, createDescriptorSetsObj(&this->graphics, &(struct DescriptorObjBuilder) {
        .layout = textureLayoutComp->descriptorSetLayout,
        .qDescriptorPoolSize = 1,
        .descriptorPoolSize = (VkDescriptorPoolSize []) {
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT
            },
        }
    }), destroyDescriptorSets);
    addResource(&this->resource, COMP_TEXT_DESCRIPTOR_FRAG, createDescriptorSetsObj(&this->graphics, &(struct DescriptorObjBuilder) {
        .layout = textureLayoutFrag->descriptorSetLayout,
        .qDescriptorPoolSize = 1,
        .descriptorPoolSize = (VkDescriptorPoolSize []) {
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT
            },
        }
    }), destroyDescriptorSets);

    struct DescriptorObj *descriptorComp = findResource(&this->resource, COMP_TEXT_DESCRIPTOR_COMP);
    struct DescriptorObj *descriptorFrag = findResource(&this->resource, COMP_TEXT_DESCRIPTOR_FRAG);

    VkDescriptorImageInfo imageInfo = {
        .sampler = image->sampler,
        .imageView = image->imageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        vkUpdateDescriptorSets(this->graphics.device, 1, (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorFrag->descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .pImageInfo = &imageInfo
            }
        }, 0, NULL);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        vkUpdateDescriptorSets(this->graphics.device, 1, (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorComp->descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = 1,
                .pImageInfo = &imageInfo
            }
        }, 0, NULL);
    }
}

void loadCompTextResources(struct EngineCore *engine, enum state *state) {
    addModelData(engine);
    addTextures(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createTextures(engine);
    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;
}
