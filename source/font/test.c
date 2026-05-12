#include "engineCore.h"
#include "state.h"

#include "defaultInstance.h"
#include "defaultCamera.h"

#include "entity.h"

#include "renderPassObj.h"
#include "commandQueue.h"

#include "fontEnum.h"

void fontTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, FONT_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, FONT_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, FONT_RENDER_PASS);
    struct ResourceManager *commandQueue = findResource(&engine->resource, FONT_COMMAND_QUEUE);

    struct Entity *entity[] = {
        findResource(entityData, FONT_ENTITIES_1)
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, FONT_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, FONT_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, FONT_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    struct CommandQueue *graphics = findResource(commandQueue, FONT_COMMAND_QUEUE_GRAPHICS);
    struct CommandQueue *queue[] = {
        graphics,
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    struct instance *floor = entity[0]->instance;
    float scale = 3 * 10e-6;

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveThirdPersonCamera(&engine->window, renderPass[0]->camera, engine->deltaTime.deltaTime);

        floor[0].scale[0] = scale;
        floor[0].scale[1] = scale;
        floor[0].scale[2] = scale;

        engineUpdate(engine, qRenderPass, renderPass);
        
        aquireNextImage(engine, graphics->inFlightFence, graphics->semaphore);

        queueDraw(graphics, engine, qRenderPass, renderPass, 1, 
            (VkSemaphore []) {
                graphics->semaphore[engine->currentFrame],
            },
            (VkPipelineStageFlags []) {
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            }
        );

        presentFrame(engine, qRenderPassArr, renderPassArr, qQueue, queue);

        if (isKeyJustPressed(&engine->window, GLFW_KEY_SPACE)) {
            state[1] = MOVE_NEXT;
        }
        else if (isKeyJustPressed(&engine->window, GLFW_KEY_N)) {
            state[1] = MOVE_NEXT_STRING;
        }
        else if (isKeyJustPressed(&engine->window, GLFW_KEY_T)) {
            state[1] = MOVE_NEXT_TEST;
        }

        if (isKeyPressed(&engine->window, GLFW_KEY_UP)) scale += 10e-8;
        if (isKeyPressed(&engine->window, GLFW_KEY_DOWN)) scale -= 10e-8;
    }
}
