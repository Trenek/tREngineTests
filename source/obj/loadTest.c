#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "instanceBuffer.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

#include "objEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, OBJ_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, OBJ_GRAPHIC_PIPELINES);

    struct graphicsPipeline *pipe[] = { 
        findResource(graphicPipelineData, OBJ_GRAPHIC_PIPELINES_1),
    };

    struct Entity *entity[] = {
        findResource(entityData, OBJ_ENTITIES_1)
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, OBJ_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, OBJ_RENDER_PASS_CLEAN);

    addResource(screenData, OBJ_SCREEN_1,
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
                .relativePos = { 100.0f, 100.0f, 100.0f },
            }
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

void loadObjTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);

    state[1] = TEST;
}
