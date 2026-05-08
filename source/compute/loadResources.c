#include <cglm/cglm.h>

#include <string.h>

#include "descriptorObj.h"
#include "bufferOperations.h"

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

static void createStorageBuffers(VkBuffer *shaderStorageBuffers, VkDeviceMemory *shaderStorageBufferMemory, struct GraphicsSetup *graphics) {
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
         // 0.000125f
    }
    VkDeviceSize bufferSize = sizeof(struct Particle) * PARTICLE_COUNT;

    VkBuffer srcBuffer = createBuffer(
        graphics->device,
        graphics->physicalDevice,
        graphics->surface,
        bufferSize * MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    );
    VkDeviceMemory srcBufferMem = createBufferMemory(
        graphics->device,
        graphics->physicalDevice,
        srcBuffer,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    void *data;

    vkMapMemory(graphics->device, srcBufferMem, 0, bufferSize, 0, &data);
    memcpy(data, part, bufferSize);
    vkUnmapMemory(graphics->device, srcBufferMem);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        shaderStorageBuffers[i] = createBuffer(
            graphics->device,
            graphics->physicalDevice,
            graphics->surface,
            bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        );
        shaderStorageBufferMemory[i] = createBufferMemory(
            graphics->device,
            graphics->physicalDevice,
            shaderStorageBuffers[i],
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        copyBuffer(srcBuffer, shaderStorageBuffers[i], bufferSize, graphics->device, graphics->transferCommandPool, graphics->transferQueue);
    }

    destroyBuffer(graphics->device, srcBuffer, srcBufferMem);
    free(part);
}

struct UniformBufferObject {
    alignas(64) float deltaTime;
};

static void createUniformBuffers(struct GraphicsSetup *graphics, VkBuffer *uniformBuffers, VkDeviceMemory *uniformBuffersMemory, void **uniformBuffersMapped) {
    VkDeviceSize bufferSize = sizeof(struct UniformBufferObject);

    *uniformBuffers = createBuffer(
        graphics->device,
        graphics->physicalDevice,
        graphics->surface,
        bufferSize * MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    );
    *uniformBuffersMemory = createBufferMemory(
        graphics->device,
        graphics->physicalDevice,
        *uniformBuffers,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    vkMapMemory(graphics->device, *uniformBuffersMemory, 0, bufferSize, 0, uniformBuffersMapped);
    for (size_t i = 1; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffersMapped[i] = (char *)uniformBuffersMapped[0] + i * bufferSize;
    }
}

struct BufferData {
    VkDevice device;

    VkBuffer storageBuffer[MAX_FRAMES_IN_FLIGHT];
    VkBuffer uniformBuffer;
    VkDeviceMemory storageMemory[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniformMemory;
};

static void destroyBufferData(void *ptr) {
    struct BufferData *bd = ptr;

    vkDestroyBuffer(bd->device, bd->uniformBuffer, NULL);
    vkFreeMemory(bd->device, bd->uniformMemory, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        vkDestroyBuffer(bd->device, bd->storageBuffer[i], NULL);
        vkFreeMemory(bd->device, bd->storageMemory[i], NULL);
    }
}

static void createMemoryBuffers(struct EngineCore *this) {
    struct ResourceManager *data = calloc(1, sizeof(struct ResourceManager));

    void **uniformMapped = malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);
    struct BufferData *bd = malloc(sizeof(struct BufferData));

    bd->device = this->graphics.device;
    createUniformBuffers(&this->graphics, &bd->uniformBuffer, &bd->uniformMemory, uniformMapped);
    createStorageBuffers(bd->storageBuffer, bd->storageMemory, &this->graphics);

    addResource(data, COMP_BUFFER_DATA_MAPPED, uniformMapped, free);
    addResource(data, COMP_BUFFER_DATA_DATA, bd, destroyBufferData);

    addResource(&this->resource, COMP_BUFFER_DATA, data, cleanupResourceManager);
}

static void createDescriptorSetss(struct EngineCore *this) {
    struct descriptorSetLayout *computeLayout = findResource(findResource(&this->resource, COMP_OBJECT_LAYOUT), COMP_OBJECT_LAYOUT_COMPUTE);
    struct BufferData *bd = findResource(findResource(&this->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_DATA);
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
            .buffer = bd->uniformBuffer,
            .offset = i * sizeof(struct UniformBufferObject),
            .range = sizeof(struct UniformBufferObject),
        };

        VkDescriptorBufferInfo storageBufferInfoLastFrame = {
            .buffer = bd->storageBuffer[(i - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT],
            .offset = 0,
            .range = sizeof(struct Particle) * PARTICLE_COUNT,
        };

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame = {
            .buffer = bd->storageBuffer[i],
            .offset = 0,
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

    struct descriptorSetLayout *compLayout = findResource(findResource(&this->resource, COMP_OBJECT_LAYOUT), COMP_OBJECT_LAYOUT_COMPUTE);

    addResource(graphicPipelinesData, COMP_GRAPHIC_PIPELINE_LAYOUT_COMP, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            compLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 1,
    }, &this->graphics), destroyPipelineLayoutObj);
    addResource(graphicPipelinesData, COMP_GRAPHIC_PIPELINE_LAYOUT_PURE, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
    }, &this->graphics), destroyPipelineLayoutObj);

    addResource(&this->resource, COMP_GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, COMP_RENDER_PASS);

    struct graphicsPipelineLayout *pipelineCompLayout = findResource(findResource(&this->resource, COMP_GRAPHIC_PIPELINE_LAYOUTS), COMP_GRAPHIC_PIPELINE_LAYOUT_COMP);
    struct graphicsPipelineLayout *pipelinePureLayout = findResource(findResource(&this->resource, COMP_GRAPHIC_PIPELINE_LAYOUTS), COMP_GRAPHIC_PIPELINE_LAYOUT_PURE);

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
    struct BufferData *bd = findResource(findResource(&this->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_DATA);

    struct DrawCall2 *a = malloc(sizeof(struct DrawCall2));
    *a = (struct DrawCall2) {
        .vertexBuffer = bd->storageBuffer,
        .verticesQuantity = PARTICLE_COUNT,
    };

    struct Entity *b = malloc(sizeof(struct Entity));
    *b = (struct Entity) {
        .drawCallQuantity = 1,
        .drawCall = a,
    };
    addResource(entityData, COMP_DRAWCALLS_1, a, free);
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
