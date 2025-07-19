#include <cglm.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "instanceBuffer.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, "Entity");
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, "graphicPipelines");

    struct graphicsPipeline *pipe[] = { 
        findResource(graphicPipelineData, "GLTF-Pipe"),
    };

    struct Entity *entity[] = {
        findResource(entityData, "Object")
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, "RenderPassCoreData");
    struct renderPassCore *clean = findResource(renderPassCoreData, "Clean");

    addResource(screenData, "Screen", 
        createRenderPassObj((struct renderPassBuilder){
            .coordinates = { 0.0, 0.0, 1.0, 1.0 },
            .color = { 0.5f, 0.5f, 0.5f, 1.0f },
            .renderPass = clean,
            .data = (struct pipelineConnection[]) {
                {
                    .pipe = pipe[0],
                    .entity = (struct Entity *[]) {
                        entity[0],
                    },
                    .qEntity = 1
                },
            },
            .qData = 1,
            .updateCameraBuffer = updateThirdPersonCameraBuffer,
            .camera.tP = {
                .center = { 0.0f, 0.0f, 0.0f },
                .relativePos = { 1.0f, 1.0f, 1.0f }
            }
        }, &engine->graphics),
        destroyRenderPassObj
    );

    struct instance *floor = entity[0]->instance;

    floor[0] = (struct instance){
        .pos = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    addResource(&engine->resource, "ScreenData", screenData, cleanupResourceManager);
}

void loadTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);

    *state = TEST;
}
