#include "engineCore.h"

#include "state.h"

void moveNextTest(struct EngineCore *engine, enum state *state) {
    vkDeviceWaitIdle(engine->graphics.device);
    cleanupResourcesOrg(&engine->resource);
    engine->resource = (struct ResourceManager) {};

    state[0] += 1;
    state[0] %= TEST_Q;
    state[1] = LOAD_RESOURCES;
}

int main() {
    struct EngineCore engine = setup("Engine Tester", "icon/ikonka.png");
    void (*const state[][STATE_Q])(struct EngineCore *engine, enum state *state) = {
        [FONT][TEST] = fontTest,
        [FONT][LOAD_TEST] = loadFontTest,
        [FONT][LOAD_RESOURCES] = loadFontResources,
        [FONT][MOVE_NEXT] = moveNextFont,
        [FONT][MOVE_NEXT_STRING] = moveNextString,
        [FONT][MOVE_NEXT_TEST] = moveNextTest,

        [OBJ][TEST] = objTest,
        [OBJ][LOAD_TEST] = loadObjTest,
        [OBJ][LOAD_RESOURCES] = loadObjResources,
        [OBJ][MOVE_NEXT] = moveNextObj,
        [OBJ][MOVE_NEXT_TEST] = moveNextTest,

        [MULTI_OBJ][TEST] = multiObjTest,
        [MULTI_OBJ][LOAD_TEST] = loadMultiObjTest,
        [MULTI_OBJ][LOAD_RESOURCES] = loadMultiObjResources,
        [MULTI_OBJ][MOVE_NEXT_TEST] = moveNextTest,

        [GLTF][TEST] = gltfTest,
        [GLTF][LOAD_TEST] = loadGltfTest,
        [GLTF][LOAD_RESOURCES] = loadGltfResources,
        [GLTF][MOVE_NEXT] = moveNextGltf,
        [GLTF][MOVE_NEXT_TEST] = moveNextTest,

        [TWO_ANIMS_GLTF][TEST] = twoAnimsGltfTest,
        [TWO_ANIMS_GLTF][LOAD_TEST] = loadTwoAnimsGltfTest,
        [TWO_ANIMS_GLTF][LOAD_RESOURCES] = loadTwoAnimsGltfResources,
        [TWO_ANIMS_GLTF][MOVE_NEXT_TEST] = moveNextTest,

        [REC][TEST] = recTest,
        [REC][LOAD_TEST] = loadRecTest,
        [REC][LOAD_RESOURCES] = loadRecResources,
        [REC][MOVE_NEXT_TEST] = moveNextTest,
    };

    enum state stateID[] = {
        TWO_ANIMS_GLTF,
        LOAD_RESOURCES
    };

    do {
        state[stateID[0]][stateID[1]](&engine, stateID);
    } while (stateID[0] != EXIT && !shouldWindowClose(engine.window));

    cleanup(engine);

    return 0;
}
