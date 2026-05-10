#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultInstance.h"
#include "defaultCamera.h"

#include "entity.h"

#include "renderPassObj.h"

#include "rectangleEnum.h"
#include "rectangle.h"
#include "commandQueue.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y)) 
#define MIN(x, y) ((y) > (x) ? (x) : (y))

#define MAX_4(x, y, z, h, i) MAX(MAX(x[i], y[i]), MAX(z[i], h[i])) 
#define MIN_4(x, y, z, h, i) MIN(MIN(x[i], y[i]), MIN(z[i], h[i]))

static void normalShadowButton(struct GraphicsSetup gs, struct WindowManager wm, struct Entity *entity, struct CameraBuffer *camera) {
    struct instance *instance = entity->instance;
    struct instanceBuffer *instanceBuffer = entity->buffer[0];

    vec2 p; {
        double pp[2];
        float scale[2];

        glfwGetCursorPos(wm.window, pp, pp + 1);
        glfwGetWindowContentScale(wm.window, scale, scale + 1); // for proper handling of different resolution monitors

        p[0] = 2 * scale[0] * pp[0] / gs.swapChain.extent.width - 1;
        p[1] = 2 * scale[1] * pp[1] / gs.swapChain.extent.height - 1;
    }

    vec3 temp[4] = {
        { -1.0f, -1.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f },
        { 1.0f, -1.0f, 0.0f },
    };

    mat4 tempMat; {
        glm_mat4_mul(camera->proj, instanceBuffer->modelMatrix, tempMat);
    }

    glm_mat4_mulv3(tempMat, temp[0], 1, temp[0]);
    glm_mat4_mulv3(tempMat, temp[1], 1, temp[1]);
    glm_mat4_mulv3(tempMat, temp[2], 1, temp[2]);
    glm_mat4_mulv3(tempMat, temp[3], 1, temp[3]);

    float left = MIN_4(temp[0], temp[1], temp[2], temp[3], 0);
    float right = MAX_4(temp[0], temp[1], temp[2], temp[3], 0);
    float down = MIN_4(temp[0], temp[1], temp[2], temp[3], 1);
    float up = MAX_4(temp[0], temp[1], temp[2], temp[3], 1);

    instance->textureIndex = p[0] > left && p[0] < right && p[1] > down && p[1] < up;
}

void recTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, REC_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, REC_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, REC_RENDER_PASS);

    struct Entity *entity[] = {
        findResource(entityData, REC_ENTITIES_1)
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, REC_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, REC_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, REC_RENDER_PASS_STAY)
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
        normalShadowButton(engine->graphics, engine->window, entity[0], renderPass[0]->cameraBuffer.buffersMapped[0]);

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
