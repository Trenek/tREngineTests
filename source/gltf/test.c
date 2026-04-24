#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "defaultCamera.h"
#include "defaultInstance.h"

#include "entity.h"

#include "renderPassObj.h"

#include "gltfEnum.h"
#include "gltf.h"

void gltfTest(struct EngineCore *engine, enum state *state) {
    struct ResourceManager *modelData = findResource(&engine->resource, GLTF_MODEL);
    struct ResourceManager *entityData = findResource(&engine->resource, GLTF_ENTITIES);
    struct ResourceManager *screenData = findResource(&engine->resource, GLTF_SCREEN);
    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, GLTF_RENDER_PASS);

    struct Entity *entity[] = {
        findResource(entityData, GLTF_ENTITIES_1)
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct Model *model[] = {
        findResource(modelData, GLTF_MODEL_1)
    };

    struct renderPassObj *renderPass[] = {
        findResource(screenData, GLTF_SCREEN_1),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, GLTF_RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, GLTF_RENDER_PASS_STAY)
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    float currentTime = 0;

    while (TEST == state[1] && !shouldWindowClose(engine->window)) {
        updateWindow(&engine->window);
    
        animate(entity[0], model[0], 0, currentTime);

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveThirdPersonCamera(&engine->window, renderPass[0]->camera, engine->deltaTime.deltaTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);

        if (isKeyJustPressed(&engine->window, GLFW_KEY_SPACE)) {
            state[1] = MOVE_NEXT;
        }

        currentTime += engine->deltaTime.deltaTime;
    }
}
