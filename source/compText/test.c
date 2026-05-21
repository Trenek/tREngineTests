#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultCamera.h"

#include "entity.h"

#include "renderPassObj.h"

#include "compTextEnum.h"
#include "rectangle.h"
#include "commandQueue.h"
#include "windowManager.h"
#include "graphicsPipelineObj.h"
#include "descriptorObj.h"

#include "commandQueue.h"
#include "computePass.h"

void compTextTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *screenData = findResource(&engine->resource, COMP_TEXT_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, COMP_TEXT_RENDER_PASS);
    struct ResourceManager *commandQueue = findResource(&engine->resource, COMP_TEXT_COMMAND_QUEUE);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, COMP_TEXT_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, COMP_TEXT_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, COMP_TEXT_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    struct Pipeline *pipeline = findResource(findResource(&engine->resource, COMP_TEXT_GRAPHIC_PIPELINES), COMP_TEXT_COMPUTE_PIPELINE);
    struct DescriptorObj *descriptorComp = findResource(&engine->resource, COMP_TEXT_DESCRIPTOR_COMP);
    struct ComputePass computePass[] = {
        {
            .pipeline = pipeline->pipeline->pipeline,
            .pipelineLayout = pipeline->pipelineLayout,

            .descriptor = descriptorComp->descriptorSets,
            .groupCountX = (engine->graphics.swapChain.extent.width + 15) / 16,
            .groupCountY = (engine->graphics.swapChain.extent.height + 15) / 16,
        }
    };
    size_t qComputePass = sizeof(computePass) / sizeof(struct ComputePass);
    struct CommandQueue *graphics = findResource(commandQueue, COMP_TEXT_COMMAND_QUEUE_GRAPHICS);
    struct CommandQueue *compute = findResource(commandQueue, COMP_TEXT_COMMAND_QUEUE_COMPUTE);
    struct CommandQueue *queue[] = {
        graphics,
        compute,
    };
    size_t qQueue = sizeof(queue) / sizeof(struct CommandQueue *);

    bool running = true;

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        struct MyBuffer {
            vec2 iResolution;
            float iTime;
            float iTimeDelta;
            vec4 pad[3];
        };

        struct MyBuffer *my = renderPass[0]->camera;

        if (isKeyJustPressed(&engine->window, GLFW_KEY_SPACE)) {
            running = !running;
        }
        if (running) {
            my->iTime += my->iTimeDelta;
            my->iTimeDelta = engine->deltaTime.deltaTime;
        }
        my->iResolution[0] = engine->graphics.swapChain.extent.width;
        my->iResolution[1] = engine->graphics.swapChain.extent.height;

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

        if (isKeyJustPressed(&engine->window, GLFW_KEY_T)) {
            state[1] = MOVE_NEXT_TEST;
        }
    }
}
