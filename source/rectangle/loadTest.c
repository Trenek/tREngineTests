#include "engineCore.h"
#include "state.h"

#include "entity.h"
#include "texture.h"

#include "defaultInstance.h"
#include "defaultCamera.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

#include "rectangleEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, REC_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, REC_GRAPHIC_PIPELINES);

    struct graphicsPipeline *pipe[] = { 
        findResource(graphicPipelineData, REC_GRAPHIC_PIPELINES_1),
    };
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&engine->resource, REC_OBJECT_LAYOUT), REC_OBJECT_LAYOUT_CAMERA);

    struct Entity *entity[] = {
        findResource(entityData, REC_ENTITIES_1)
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, REC_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, REC_RENDER_PASS_CLEAN);
    struct Textures *colorTexture = findResource(findResource(&engine->resource, REC_TEXTURES), REC_TEXTURE_1);

    addResource(screenData, REC_SCREEN_1,
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
                    },
                    .qEntity = 1
                },
            },
            .qData = 1,
            .updateCameraBuffer = updateThirdPersonCameraBuffer,
            .cameraSize = sizeof(struct ThirdPerson),
            .cameraBufferSize = sizeof(struct CameraBuffer),
            .camera = &(struct ThirdPerson){
            },
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout
        }, &engine->graphics),
        destroyRenderPassObj
    );

    struct instance *params = entity[0]->instance;

    params[0] = (struct instance){
        .pos = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 0.2f, 0.1f, 0.2f },
        .textureIndex = 0
    };

    addResource(&engine->resource, REC_SCREEN, screenData, cleanupResourceManager);
}

void loadRecTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);

    state[1] = TEST;
}
