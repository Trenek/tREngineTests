#include "engineCore.h"
#include "state.h"

void moveNext(struct EngineCore *engine, enum state *state) {
    vkDeviceWaitIdle(engine->graphics.device);
    cleanupResourcesOrg(&engine->resource);
    engine->resource = (struct ResourceManager) {};

    *state = LOAD_RESOURCES;
}
