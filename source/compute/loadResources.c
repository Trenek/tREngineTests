#include <cglm/cglm.h>

#include <string.h>

#include "descriptorObj.h"

#include "engineCore.h"
#include "state.h"
#include "texture.h"

#include "model.h"
#include "entity.h"
#include "rectangleBuilder.h"
#include "defaultInstance.h"
#include "defaultCamera.h"

#include "renderPassCore.h"

#include "graphicsPipelineLayout.h"
#include "graphicsPipelineObj.h"
#include "descriptorSetLayoutObj.h"
#include "bufferObj.h"

#include "rectangle.h"

#include "compEnum.h"

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, COMP_RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, COMP_RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, COMP_RENDER_PASS, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, COMP_OBJECT_LAYOUT_CAMERA, 
        defaultCameraDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, COMP_OBJECT_LAYOUT_COMPUTE, createDescriptorSetLayoutObj(3, (VkDescriptorSetLayoutBinding[]) {
        [0] = {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImmutableSamplers = NULL,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        },
        [1] = {
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImmutableSamplers = NULL,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        },
        [2] = {
            .binding = 2,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImmutableSamplers = NULL,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        },
    }, this->graphics.device), destroyDescriptorSetLayout);

    addResource(&this->resource, COMP_OBJECT_LAYOUT, objectLayoutData, cleanupResourceManager);
}

struct Particle {
    vec2 position;
    vec2 velocity;
    vec4 color;
};

#define PARTICLE_COUNT 8'192'000

static float getRand() {
    return fabs((rand() % 1'000'000) / 1'000'000.0);
}

struct BufferObj *createStorageBuffers(struct GraphicsSetup *graphics) {
    struct BufferObj *storageBuffer = NULL;
    srand(time(NULL));

    struct Particle *part = malloc(sizeof(struct Particle) * PARTICLE_COUNT);
    for (size_t i = 0; i < PARTICLE_COUNT; i += 1) {
        const float r = 0.25f * sqrt(getRand());
        const float theta = getRand() * 2.0f * M_PI;
        const float x = r * cos(theta) * 2;
        const float y = r * sin(theta) * 2;

        part[i] = (struct Particle) {
            .position = { x, y },
            .color = { getRand(), getRand(), getRand(), 1.0f },
        };

        glm_vec2_scale(part[i].position, glm_vec2_norm(part[i].position) * 0.00025f, part[i].velocity);
    }

    struct BufferObj *srcBuffer = createBufferObj((struct BufferBuilder) {
        .bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .size = sizeof(struct Particle) * PARTICLE_COUNT,
        .repetitions = 1,
    }, graphics);

    void *data;

    vkMapMemory(graphics->device, srcBuffer->memory, 0, srcBuffer->range, 0, &data);
    memcpy(data, part, srcBuffer->range);
    vkUnmapMemory(graphics->device, srcBuffer->memory);

    storageBuffer = createBufferObj((struct BufferBuilder) {
        .bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .size = sizeof(struct Particle) * PARTICLE_COUNT,
        .repetitions = MAX_FRAMES_IN_FLIGHT,
        .value = PARTICLE_COUNT
    }, graphics);

    VkBufferCopy copyCommands[MAX_FRAMES_IN_FLIGHT]; for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        copyCommands[i] = (VkBufferCopy) {
            .srcOffset = 0,
            .dstOffset = storageBuffer->range * i,
            .size = srcBuffer->range
        };
    }

    copyBufferObj(storageBuffer, srcBuffer, MAX_FRAMES_IN_FLIGHT, copyCommands, graphics);

    destroyBufferObj(srcBuffer);
    free(part);

    return storageBuffer;
}

struct UniformBufferObject {
    float deltaTime;
};

static void createMemoryBuffers(struct EngineCore *this) {
    struct ResourceManager *data = calloc(1, sizeof(struct ResourceManager));

    addResource(data, COMP_BUFFER_DATA_UNIFORM, createBufferObj((struct BufferBuilder) {
        .bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .repetitions = MAX_FRAMES_IN_FLIGHT,
        .size = sizeof(struct UniformBufferObject),
    }, &this->graphics), destroyBufferObj);
    addResource(data, COMP_BUFFER_DATA_STORAGE, createStorageBuffers(&this->graphics), destroyBufferObj);

    addResource(&this->resource, COMP_BUFFER_DATA, data, cleanupResourceManager);
}

static void createDescriptorSetss(struct EngineCore *this) {
    struct DescriptorSetLayout *computeLayout = findResource(findResource(&this->resource, COMP_OBJECT_LAYOUT), COMP_OBJECT_LAYOUT_COMPUTE);
    struct BufferObj *uniform = findResource(findResource(&this->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_UNIFORM);
    struct BufferObj *storage = findResource(findResource(&this->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_STORAGE);
    addResource(&this->resource, COMP_DESCRIPTOR, createDescriptorSetsObj(&this->graphics, &(struct DescriptorObjBuilder) {
        .layout = computeLayout->descriptorSetLayout,
        .qDescriptorPoolSize = 2,
        .descriptorPoolSize = (VkDescriptorPoolSize []) {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT * 2
            },
        }
    }), destroyDescriptorSets);

    struct DescriptorObj *descriptor = findResource(&this->resource, COMP_DESCRIPTOR);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        VkDescriptorBufferInfo uniformBufferInfo = {
            .buffer = uniform->buffer,
            .offset = i * uniform->range,
            .range = sizeof(struct UniformBufferObject),
        };

        VkDescriptorBufferInfo storageBufferInfoLastFrame = {
            .buffer = storage->buffer,
            .offset = ((i - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT) * storage->range,
            .range = sizeof(struct Particle) * PARTICLE_COUNT,
        };

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame = {
            .buffer = storage->buffer,
            .offset = i * storage->range,
            .range = sizeof(struct Particle) * PARTICLE_COUNT,
        };

        vkUpdateDescriptorSets(this->graphics.device, 3, (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor->descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &uniformBufferInfo,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor->descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &storageBufferInfoLastFrame,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor->descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &storageBufferInfoCurrentFrame,
            }
        }, 0, NULL);
    }
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct DescriptorSetLayout *compLayout = findResource(findResource(&this->resource, COMP_OBJECT_LAYOUT), COMP_OBJECT_LAYOUT_COMPUTE);

    addResource(graphicPipelinesData, COMP_GRAPHIC_PIPELINE_LAYOUT_COMP, createPipelineLayout((struct PipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            compLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 1,
    }, &this->graphics), destroyPipelineLayoutObj);
    addResource(graphicPipelinesData, COMP_GRAPHIC_PIPELINE_LAYOUT_PURE, createPipelineLayout((struct PipelineLayoutBuilder) {
    }, &this->graphics), destroyPipelineLayoutObj);

    addResource(&this->resource, COMP_GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, COMP_RENDER_PASS);

    struct PipelineLayout *pipelineCompLayout = findResource(findResource(&this->resource, COMP_GRAPHIC_PIPELINE_LAYOUTS), COMP_GRAPHIC_PIPELINE_LAYOUT_COMP);
    struct PipelineLayout *pipelinePureLayout = findResource(findResource(&this->resource, COMP_GRAPHIC_PIPELINE_LAYOUTS), COMP_GRAPHIC_PIPELINE_LAYOUT_PURE);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, COMP_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, COMP_RENDER_PASS_STAY)
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, COMP_GRAPHIC_PIPELINE_PURE, createGraphicsPipelineObj((struct GraphicsPipelineBuilder) {
        .pipelineLayout = pipelinePureLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/compVert.spv",
        .fragmentShader = "shaders/compFrag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .vert = {
            .sizeOfVertex = sizeof(struct Particle),
            .numOfAttributes = 2,
            .attributeDescription = (VkVertexInputAttributeDescription []) {
                [0] = {
                    .binding = 0,
                    .location = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(struct Particle, position)
                },
                [1] = {
                    .binding = 0,
                    .location = 1,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .offset = offsetof(struct Particle, color)
                },
            },
        },
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyPipelineObj);
    addResource(graphicPipelinesData, COMP_GRAPHIC_PIPELINE_COMP, createComputePipelineObj((struct ComputePipelineBuilder) {
        .pipelineLayout = pipelineCompLayout->pipelineLayout,
        .computeShader = "shaders/comp.spv",
    }, &this->graphics), destroyPipelineObj);

    addResource(&this->resource, COMP_GRAPHIC_PIPELINES, graphicPipelinesData, cleanupResourceManager);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct BufferData *storage = findResource(findResource(&this->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_STORAGE);

    struct Entity *b = malloc(sizeof(struct Entity));
    *b = (struct Entity) {
        .drawCallQuantity = 1,
        .drawCall = storage,
    };
    addResource(entityData, COMP_ENTITIES_1, b, free);

    addResource(&this->resource, COMP_ENTITIES, entityData, cleanupResourceManager);
}

void loadCompResources(struct EngineCore *engine, enum state *state) {
    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createMemoryBuffers(engine);
    createDescriptorSetss(engine);

    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);
    addEntities(engine);

    state[1] = LOAD_TEST;
}
