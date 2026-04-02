#include "engineCore.h"
#include "state.h"
#include "actualModel.h"

#include "entity.h"
#include "instanceBuffer.h"

#include "renderPassObj.h"

#include "fontEnum.h"

void fontTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, FONT_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, FONT_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, FONT_RENDER_PASS);

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

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        updateWindow(&engine->window);

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveThirdPersonCamera(&engine->window, &renderPass[0]->camera, engine->deltaTime.deltaTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);

        if (isKeyJustPressed(&engine->window, GLFW_KEY_SPACE)) {
            state[1] = MOVE_NEXT;
        }
        else if (isKeyJustPressed(&engine->window, GLFW_KEY_N)) {
            state[1] = MOVE_NEXT_STRING;
        }
    }
}
