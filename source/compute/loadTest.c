#include "engineCore.h"
#include "state.h"

#include "entity.h"
#include "texture.h"

#include "defaultInstance.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"
#include "commandQueue.h"

#include "compEnum.h"

static void createScreens(struct EngineCore *engine) {
    struct ResourceManager *entityData = findResource(&engine->resource, COMP_ENTITIES);
    struct ResourceManager *graphicPipelineData = findResource(&engine->resource, COMP_GRAPHIC_PIPELINES);

    struct Pipeline *pipe[] = { 
        findResource(graphicPipelineData, COMP_GRAPHIC_PIPELINE_PURE),
    };

    struct Entity *entity[] = {
        findResource(entityData, COMP_ENTITIES_1)
    };
    
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, COMP_RENDER_PASS);
    struct renderPassCore *clean = findResource(renderPassCoreData, COMP_RENDER_PASS_CLEAN);

    addResource(screenData, COMP_SCREEN_2,
        createRenderPassObj((struct renderPassBuilder){
            .coordinates = { 0.0, 0.0, 1.0, 1.0 },
            .color = { 0.0f, 0.0f, 0.0f, 1.0f },
            .renderPass = clean,
            .data = (struct pipelineConnectionBuilder[]) {
                {
                    .pipe = pipe[0],
                    .entity = entity,
                    .qEntity = 1
                },
            },
            .qData = 1,
            .drawRenderPass = drawRenderPassComp,
        }, &engine->graphics),
        destroyRenderPassObj
    );

    addResource(&engine->resource, COMP_SCREEN, screenData, cleanupResourceManager);
}

static void createCommandQueues(struct EngineCore *engine) {
    struct ResourceManager *queueData = calloc(1, sizeof(struct ResourceManager));

    addResource(queueData, COMP_COMMAND_QUEUE_GRAPHICS, createCommandQueue(&engine->graphics), destroyCommandQueue);
    addResource(queueData, COMP_COMMAND_QUEUE_COMPUTE, createCommandQueue(&engine->graphics), destroyCommandQueue);

    addResource(&engine->resource, COMP_COMMAND_QUEUE, queueData, cleanupResourceManager);
}

void loadCompTest(struct EngineCore *engine, enum state *state) {
    createScreens(engine);
    createCommandQueues(engine);

    state[1] = TEST;
}
