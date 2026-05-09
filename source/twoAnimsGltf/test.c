#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultCamera.h"
#include "defaultInstance.h"

#include "entity.h"

#include "renderPassObj.h"

#include "twoAnimsGltfEnum.h"
#include "commandQueue.h"
#include "gltf.h"

void twoAnimsGltfTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *modelData = findResource(&engine->resource, TWO_ANIMS_GLTF_MODEL);
    struct ResourceManager *entityData = findResource(&engine->resource, TWO_ANIMS_GLTF_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, TWO_ANIMS_GLTF_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, TWO_ANIMS_GLTF_RENDER_PASS);

    struct Entity *entity[] = {
        findResource(entityData, TWO_ANIMS_GLTF_ENTITIES_1),
        findResource(entityData, TWO_ANIMS_GLTF_ENTITIES_2),
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct Model *model[] = {
        findResource(modelData, TWO_ANIMS_GLTF_MODEL_1)
    };

    struct renderPassObj *renderPass[] = {
        findResource(screenData, TWO_ANIMS_GLTF_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, TWO_ANIMS_GLTF_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, TWO_ANIMS_GLTF_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    float currentTime = 0;

    struct CommandQueue graphics;
    struct CommandQueue *queue[] = {
        &graphics,
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    createCommandQueue(&graphics, &engine->graphics);

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        animate(entity[0], model[0], 0, currentTime);
        animate(entity[1], model[0], 1, currentTime);

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveThirdPersonCamera(&engine->window, renderPass[0]->camera, engine->deltaTime.deltaTime);

        engineUpdate(engine, qRenderPass, renderPass);
        
        aquireNextImage(engine, graphics.inFlightFence, graphics.semaphore);

        queueDraw(&graphics, engine, qRenderPass, renderPass, 1, 
            (VkSemaphore []) {
                graphics.semaphore[engine->currentFrame],
            },
            (VkPipelineStageFlags []) {
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            }
        );

        presentFrame(engine, qRenderPassArr, renderPassArr, qQueue, queue);

        if (isKeyJustPressed(&engine->window, GLFW_KEY_T)) {
            state[1] = MOVE_NEXT_TEST;
        }

        currentTime += engine->deltaTime.deltaTime;
    }

    destroyCommandQueue(&graphics, &engine->graphics);
}
