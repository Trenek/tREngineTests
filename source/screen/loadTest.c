#include <string.h>

#include "engineCore.h"
#include "state.h"

#include "entity.h"
#include "texture.h"

#include "defaultInstance.h"
#include "defaultCamera.h"
#include "descriptorSetLayoutObj.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"
#include "commandQueue.h"

#include "screenEnum.h"

struct MyBuffer {
    vec2 iResolution;
    float iTime;
    float iTimeDelta;
    vec4 pad[3];
};

void updateMyBuffer(void *uniformBuffersMapped, VkExtent2D, void *cameraPtr) {
    memcpy(uniformBuffersMapped, cameraPtr, sizeof(struct MyBuffer));
}

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, SCREEN_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, SCREEN_GRAPHIC_PIPELINES);

    struct Pipeline *pipe[] = { 
        findResource(graphicPipelineData, SCREEN_GRAPHIC_PIPELINES_1),
    };
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&engine->resource, SCREEN_OBJECT_LAYOUT), SCREEN_OBJECT_LAYOUT_CAMERA);

    struct Entity *entity[] = {
        findResource(entityData, SCREEN_ENTITIES_1)
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, SCREEN_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, SCREEN_RENDER_PASS_CLEAN);

    addResource(screenData, SCREEN_SCREEN_1,
        createRenderPassObj((struct renderPassBuilder){
            .coordinates = { 0.0, 0.0, 1.0, 1.0 },
            .color = { 0.5f, 0.5f, 0.5f, 1.0f },
            .renderPass = clean,
            .data = (struct pipelineConnectionBuilder[]) {
                {
                    .pipe = pipe[0],
                    .entity = (struct Entity *[]) {
                        entity[0],
                    },
                    .qEntity = 1
                },
            },
            .qData = 1,
            .camera = {
                .updateBuffer = updateMyBuffer,
                .size = sizeof(struct MyBuffer),
                .bufferSize = sizeof(struct MyBuffer),
                .mapped = &(struct MyBuffer) {
                    .iResolution = { 
                        engine->graphics.swapChain.extent.width, 
                        engine->graphics.swapChain.extent.height, 
                    },
                    .iTime = 0,
                    .iTimeDelta = 0
                }
            },

            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout,
            .drawRenderPass = drawRenderPass,
        }, &engine->graphics),
        destroyRenderPassObj
    );

    addResource(&engine->resource, SCREEN_SCREEN, screenData, cleanupResourceManager);
}

static void createCommandQueues(struct EngineCore *engine) {
    struct ResourceManager *queueData = calloc(1, sizeof(struct ResourceManager));

    addResource(queueData, SCREEN_COMMAND_QUEUE_GRAPHICS, createCommandQueue(&engine->graphics), destroyCommandQueue);

    addResource(&engine->resource, SCREEN_COMMAND_QUEUE, queueData, cleanupResourceManager);
}

void loadScreenTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);
    createCommandQueues(engine);

    state[1] = TEST;
}
