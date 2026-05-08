#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultCamera.h"
#include "defaultInstance.h"

#include "entity.h"

#include "renderPassObj.h"
#include "commandQueue.h"

#include "objEnum.h"

void objTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, OBJ_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, OBJ_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, OBJ_RENDER_PASS);

    struct Entity *entity[] = {
        findResource(entityData, OBJ_ENTITIES_1)
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, OBJ_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, OBJ_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, OBJ_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    struct CommandQueue graphics;
    struct CommandQueue *queue[] = {
        &graphics,
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    createCommandQueue(&graphics, &engine->graphics);

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
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

        if (isKeyJustPressed(&engine->window, GLFW_KEY_SPACE)) {
            state[1] = MOVE_NEXT;
        }
        else if (isKeyJustPressed(&engine->window, GLFW_KEY_T)) {
            state[1] = MOVE_NEXT_TEST;
        }
    }

    destroyCommandQueue(&graphics, &engine->graphics);
}
