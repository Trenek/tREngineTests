#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "instanceBuffer.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

#include "multiObjEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, MULTI_OBJ_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, MULTI_OBJ_GRAPHIC_PIPELINES);

    struct graphicsPipeline *pipe[] = { 
        findResource(graphicPipelineData, MULTI_OBJ_GRAPHIC_PIPELINES_1),
    };
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&engine->resource, MULTI_OBJ_OBJECT_LAYOUT), MULTI_OBJ_OBJECT_LAYOUT_CAMERA);

    struct Entity *entity[] = {
        findResource(entityData, 0),
        findResource(entityData, 1),
        findResource(entityData, 2),
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, MULTI_OBJ_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, MULTI_OBJ_RENDER_PASS_CLEAN);
    struct Textures *colorTexture = findResource(findResource(&engine->resource, MULTI_OBJ_TEXTURES), MULTI_OBJ_TEXTURE_1);

    addResource(screenData, MULTI_OBJ_SCREEN_1,
        createRenderPassObj((struct renderPassBuilder){
            .coordinates = { 0.0, 0.0, 1.0, 1.0 },
            .color = { 0.5f, 0.5f, 0.5f, 1.0f },
            .renderPass = clean,
            .data = (struct pipelineConnection[]) {
                {
                    .texture = &colorTexture->descriptor,
                    .pipe = pipe[0],
                    .entity = (struct Entity *[]) {
                        entity[0],
                        entity[1],
                        entity[2],
                    },
                    .qEntity = 3
                },
            },
            .qData = 1,
            .updateCameraBuffer = updateThirdPersonCameraBuffer,
            .camera = {
                .center = { 0.0f, 0.0f, 0.0f },
                .relativePos = { 2.0f, 2.0f, 2.0f },
            },
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout
        }, &engine->graphics),
        destroyRenderPassObj
    );

    struct instance *params[] = {
        entity[0]->instance,
        entity[1]->instance,
        entity[2]->instance,
    };

    params[0][0] = (struct instance){
        .pos = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    params[1][0] = (struct instance){
        .pos = { 2.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    params[2][0] = (struct instance){
        .pos = { -2.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    addResource(&engine->resource, MULTI_OBJ_SCREEN, screenData, cleanupResourceManager);
}

void loadMultiObjTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);

    state[1] = TEST;
}
