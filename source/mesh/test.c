#include <cglm.h>

#include "engineCore.h"
#include "state.h"
#include "actualModel.h"

#include "entity.h"
#include "instanceBuffer.h"

#include "renderPassObj.h"

void meshTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, "Entity");
    struct ResourceManager *screenData = findResource(&engine->resource, "ScreenData");
    struct ResourceManager *modelData = findResource(&engine->resource, "modelData");

    struct Entity *entity[] = {
        findResource(entityData, "Object")
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    [[maybe_unused]]
    struct actualModel *model[] = {
        findResource(modelData, "Box")
    };
    
    struct renderPassObj *renderPass[] = {
        findResource(screenData, "Screen"),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, "RenderPassCoreData");
    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    float time = 0;

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        updateWindow(&engine->window);

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveThirdPersonCamera(&engine->window, &renderPass[0]->camera, engine->deltaTime.deltaTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);
        animate(entity[0], model[0], 0, time);

        time += engine->deltaTime.deltaTime;
        if (isKeyJustPressed(&engine->window, GLFW_KEY_SPACE)) {
            state[1] = MOVE_NEXT;
        }
    }
}
