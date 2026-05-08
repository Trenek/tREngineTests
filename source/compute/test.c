#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultCamera.h"
#include "entity.h"

#include "renderPassObj.h"
#include "graphicsPipelineObj.h"
#include "descriptorObj.h"
#include "computePass.h"

#include "compEnum.h"
#include "rectangle.h"

#include "graphicsFunctions.h"

#include "commandQueue.h"

void compTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *screenData = findResource(&engine->resource, COMP_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, COMP_RENDER_PASS);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, COMP_SCREEN_2),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, COMP_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, COMP_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);
    struct UniformBufferObject {
        float deltaTime;
    };

    struct UniformBufferObject **time = findResource(findResource(&engine->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_MAPPED);
    struct DescriptorObj *descriptor = findResource(&engine->resource, 8);
    struct Pipeline *pipeline = findResource(findResource(&engine->resource, 4), 1);

    struct DrawCall2 *dc = findResource(findResource(&engine->resource, COMP_ENTITIES), COMP_DRAWCALLS_1);
    struct ComputePass computePass[] = {
        {
            .pipeline = pipeline->pipeline->pipeline,
            .pipelineLayout = pipeline->pipelineLayout,

            .descriptor = descriptor->descriptorSets,
            .groupCountX = dc->verticesQuantity / 256
        }
    };
    size_t qComputePass = sizeof(computePass) / sizeof(struct ComputePass);

    struct CommandQueue graphics;
    struct CommandQueue compute;

    struct CommandQueue *queue[] = {
        &graphics,
        &compute,
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    createCommandQueue(&graphics, &engine->graphics);
    createCommandQueue(&compute, &engine->graphics);

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        moveThirdPersonCamera(&engine->window, renderPass[0]->camera, engine->deltaTime.deltaTime);
        engineUpdate(engine, qRenderPass, renderPass);

        aquireNextImage(engine, graphics.inFlightFence, graphics.semaphore);

        queueCompute(&compute, engine, qComputePass, computePass);
        queueDraw(&graphics, engine, qRenderPass, renderPass, 2, 
            (VkSemaphore []) {
                compute.semaphore[engine->currentFrame],
                graphics.semaphore[engine->currentFrame],
            },
            (VkPipelineStageFlags []) {
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            }
        );

        presentFrame(engine, qRenderPassArr, renderPassArr, qQueue, queue);

        time[engine->currentFrame]->deltaTime =
            (false == isKeyPressed(&engine->window, GLFW_KEY_SPACE)) ?
                isKeyPressed(&engine->window, GLFW_KEY_B) ? 1'000'000'000 * engine->deltaTime.deltaTime :
                isKeyPressed(&engine->window, GLFW_KEY_N) ? 1'000'000 * engine->deltaTime.deltaTime :
                isKeyPressed(&engine->window, GLFW_KEY_M) ? 1'000 * engine->deltaTime.deltaTime :
                                                            1'000'000
            : 0;

        if (isKeyJustPressed(&engine->window, GLFW_KEY_T)) {
            state[1] = MOVE_NEXT_TEST;
        }
    }

    destroyCommandQueue(&graphics, &engine->graphics);
    destroyCommandQueue(&compute, &engine->graphics);
}
