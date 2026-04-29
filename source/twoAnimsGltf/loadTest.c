#include "engineCore.h"
#include "state.h"

#include "texture.h"
#include "entity.h"

#include "defaultInstance.h"
#include "defaultCamera.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

#include "twoAnimsGltfEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, TWO_ANIMS_GLTF_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, TWO_ANIMS_GLTF_GRAPHIC_PIPELINES);

    struct graphicsPipeline *pipe[] = { 
        findResource(graphicPipelineData, TWO_ANIMS_GLTF_GRAPHIC_PIPELINES_1),
    };
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&engine->resource, TWO_ANIMS_GLTF_OBJECT_LAYOUT), TWO_ANIMS_GLTF_OBJECT_LAYOUT_CAMERA);

    struct Entity *entity[] = {
        findResource(entityData, TWO_ANIMS_GLTF_ENTITIES_1),
        findResource(entityData, TWO_ANIMS_GLTF_ENTITIES_2),
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, TWO_ANIMS_GLTF_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, TWO_ANIMS_GLTF_RENDER_PASS_CLEAN);
    struct Textures *colorTexture = findResource(findResource(&engine->resource, TWO_ANIMS_GLTF_TEXTURES), TWO_ANIMS_GLTF_TEXTURE_1);

    addResource(screenData, TWO_ANIMS_GLTF_SCREEN_1,
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
                    },
                    .qEntity = 2
                },
            },
            .qData = 1,
            .camera = defaultThirdPersonCamera(&(struct ThirdPerson) {
                .center = { 0.0f, 0.0f, 0.0f },
                .relativePos = { 0.0f, 0.0f, 2.0f },
            }),
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout
        }, &engine->graphics),
        destroyRenderPassObj
    );

    struct instance *params1 = entity[0]->instance;
    struct instance *params2 = entity[1]->instance;

    params1[0] = (struct instance){
        .pos = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    params2[0] = (struct instance){
        .pos = { 6.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
        .textureIndex = 0
    };

    addResource(&engine->resource, TWO_ANIMS_GLTF_SCREEN, screenData, cleanupResourceManager);
}

void loadTwoAnimsGltfTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);

    state[1] = TEST;
}
