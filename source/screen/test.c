#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultInstance.h"
#include "defaultCamera.h"

#include "entity.h"

#include "renderPassObj.h"

#include "screenEnum.h"
#include "rectangle.h"
#include "commandQueue.h"

void screenTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, SCREEN_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, SCREEN_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, SCREEN_RENDER_PASS);

    struct Entity *entity[] = {
        findResource(entityData, SCREEN_ENTITIES_1)
    };
    [[maybe_unused]]
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, SCREEN_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, SCREEN_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, SCREEN_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    struct CommandQueue graphics;
    struct CommandQueue *queue[] = {
        &graphics,
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    createCommandQueue(&graphics, &engine->graphics);

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        struct MyBuffer {
            vec2 iResolution;
            float iTime;
            float iTimeDelta;
            vec4 pad[3];
        };

        struct MyBuffer *my = renderPass[0]->camera;

        my->iTimeDelta = engine->deltaTime.deltaTime;
        my->iTime += my->iTimeDelta;
        my->iResolution[0] = engine->graphics.swapChain.extent.width;
        my->iResolution[1] = engine->graphics.swapChain.extent.height;

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
    }

    destroyCommandQueue(&graphics, &engine->graphics);
}
