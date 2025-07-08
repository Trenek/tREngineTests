#include <cglm.h>

#include "engineCore.h"
#include "state.h"

#include "entity.h"
#include "instanceBuffer.h"

#include "renderPassObj.h"

void test(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *entityData = findResource(&engine->resource, "Entity");
    struct ResourceManager *screenData = findResource(&engine->resource, "ScreenData");

    struct Entity *entity[] = {
        findResource(entityData, "Object")
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);
    
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

    while (TEST == *state && !shouldWindowClose(engine->window)) {
        glfwPollEvents();

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);

        if ((KEY_PRESS | KEY_CHANGE) == getKeyState(&engine->window, GLFW_KEY_SPACE)) {
            *state = MOVE_NEXT;
        }
    }
}
