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

#include "objEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, OBJ_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, OBJ_GRAPHIC_PIPELINES);

    struct Pipeline *pipe[] = { 
        findResource(graphicPipelineData, OBJ_GRAPHIC_PIPELINES_1),
    };
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&engine->resource, OBJ_OBJECT_LAYOUT), OBJ_OBJECT_LAYOUT_CAMERA);

    struct Entity *entity[] = {
        findResource(entityData, OBJ_ENTITIES_1)
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, OBJ_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, OBJ_RENDER_PASS_CLEAN);
    struct Textures *colorTexture = findResource(findResource(&engine->resource, OBJ_TEXTURES), OBJ_TEXTURE_1);

    addResource(screenData, OBJ_SCREEN_1,
        createRenderPassObj((struct renderPassBuilder){
            .coordinates = { 0.0, 0.0, 1.0, 1.0 },
            .color = { 0.5f, 0.5f, 0.5f, 1.0f },
            .renderPass = clean,
            .data = (struct pipelineConnectionBuilder[]) {
                {
                    .texture = &colorTexture->descriptor,
                    .pipe = pipe[0],
                    .entity = (struct Entity *[]) {
                        entity[0],
                    },
                    .qEntity = 1
                },
            },
            .qData = 1,
            .camera = defaultThirdPersonCamera(&(struct ThirdPerson) {
                .center = { 0.0f, 0.0f, 0.0f },
                .relativePos = { 2.0f, 2.0f, 2.0f },
            }),
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout,
            .drawRenderPass = drawRenderPass,
        }, &engine->graphics),
        destroyRenderPassObj
    );

    struct instance *params = entity[0]->instance;

    params[0] = (struct instance){
        .pos = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    addResource(&engine->resource, OBJ_SCREEN, screenData, cleanupResourceManager);
}

static void createCommandQueues(struct EngineCore *engine) {
    struct ResourceManager *queueData = calloc(1, sizeof(struct ResourceManager));

    addResource(queueData, OBJ_COMMAND_QUEUE_GRAPHICS, createCommandQueue(&engine->graphics), destroyCommandQueue);

    addResource(&engine->resource, OBJ_COMMAND_QUEUE, queueData, cleanupResourceManager);
}

void loadObjTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);
    createCommandQueues(engine);

    state[1] = TEST;
}
