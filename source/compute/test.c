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
#include "bufferObj.h"

#include "commandQueue.h"

void compTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *screenData = findResource(&engine->resource, COMP_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, COMP_RENDER_PASS);
    struct ResourceManager *commandQueue = findResource(&engine->resource, COMP_COMMAND_QUEUE);

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

    struct BufferObj *uniform = findResource(findResource(&engine->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_UNIFORM);
    struct UniformBufferObject *time[MAX_FRAMES_IN_FLIGHT] = {};
    vkMapMemory(engine->graphics.device, uniform->memory, 0, uniform->range, 0, (void **)&time[0]);
    for (size_t i = 1; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        time[i] = (void *)((char *)time[i - 1] + uniform->range);
    }
    struct DescriptorObj *descriptor = findResource(&engine->resource, COMP_DESCRIPTOR);
    struct Pipeline *pipeline = findResource(findResource(&engine->resource, COMP_GRAPHIC_PIPELINES), COMP_GRAPHIC_PIPELINE_COMP);

    struct BufferObj *storage = findResource(findResource(&engine->resource, COMP_BUFFER_DATA), COMP_BUFFER_DATA_STORAGE);

    struct ComputePass computePass[] = {
        {
            .pipeline = pipeline->pipeline->pipeline,
            .pipelineLayout = pipeline->pipelineLayout,

            .descriptor = descriptor->descriptorSets,
            .groupCountX = storage->value / 256,
            .groupCountY = 1,
        }
    };
    size_t qComputePass = sizeof(computePass) / sizeof(struct ComputePass);

    struct CommandQueue *graphics = findResource(commandQueue, COMP_COMMAND_QUEUE_GRAPHICS);
    struct CommandQueue *compute = findResource(commandQueue, COMP_COMMAND_QUEUE_COMPUTE);
    struct CommandQueue *queue[] = {
        graphics,
        compute
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        engineUpdate(engine, qRenderPass, renderPass);

        aquireNextImage(engine, graphics->inFlightFence, graphics->semaphore);

        queueCompute(compute, engine, qComputePass, computePass);
        queueDraw(graphics, engine, qRenderPass, renderPass, 2, 
            (VkSemaphore []) {
                compute->semaphore[engine->currentFrame],
                graphics->semaphore[engine->currentFrame],
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

    vkUnmapMemory(graphics->device, uniform->memory);
}
