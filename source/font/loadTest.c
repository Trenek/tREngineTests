#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "instanceBuffer.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

#include "fontEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, FONT_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, FONT_GRAPHIC_PIPELINES);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, FONT_RENDER_PASS);

    struct descriptorSetLayout *cameraLayout = findResource(findResource(&engine->resource, FONT_OBJECT_LAYOUT), FONT_OBJECT_LAYOUT_CAMERA);
    struct graphicsPipeline *pipe[] = { 
        findResource(graphicPipelineData, FONT_GRAPHIC_PIPELINES_1),
    };
    struct Entity *entity[] = {
        findResource(entityData, FONT_ENTITIES_1)
    };
    struct renderPassCore *clean = findResource(renderPassCoreData, FONT_RENDER_PASS_CLEAN);

    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    addResource(screenData, FONT_SCREEN_1,
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
            .cameraSize = sizeof(struct ThirdPerson),
            .updateCameraBuffer = updateThirdPersonCameraBuffer,
            .camera = &(struct ThirdPerson){
                .center = { 0.0f, 0.0f, 0.0f },
                .relativePos = { -2.0f, 0.0f, 0.0f }
            },
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout
        }, &engine->graphics),
        destroyRenderPassObj
    );

    struct instance *floor = entity[0]->instance;

    floor[0] = (struct instance){
        .pos = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 3 * 10e-6, 3 * 10e-6, 3 * 10e-6 },
        .textureIndex = 0
    };

    addResource(&engine->resource, FONT_SCREEN, screenData, cleanupResourceManager);
}

void loadFontTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);

    state[1] = TEST;
}
