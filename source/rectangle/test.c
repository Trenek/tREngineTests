#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"
#include "actualModel.h"

#include "entity.h"
#include "instanceBuffer.h"

#include "renderPassObj.h"

#include "rectangleEnum.h"

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

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        updateWindow(&engine->window);

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveThirdPersonCamera(&engine->window, renderPass[0]->camera, engine->deltaTime.deltaTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);
    }
}
