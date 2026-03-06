#include "engineCore.h"

#include "state.h"

int main() {
    struct EngineCore engine = setup("Engine Tester", "icon/ikonka.png");
    void (*const state[][STATE_Q])(struct EngineCore *engine, enum state *state) = {
        [MESH][TEST] = meshTest,
        [MESH][LOAD_TEST] = loadMeshTest,
        [MESH][LOAD_RESOURCES] = loadMeshResources,
        [MESH][MOVE_NEXT] = moveNextMesh,

        [FONT][TEST] = fontTest,
        [FONT][LOAD_TEST] = loadFontTest,
        [FONT][LOAD_RESOURCES] = loadFontResources,
        [FONT][MOVE_NEXT] = moveNextFont,
        [FONT][MOVE_NEXT_STRING] = moveNextString,
    };

    enum state stateID[] = {
        MESH,
        LOAD_RESOURCES
    };

    do {
        state[stateID[0]][stateID[1]](&engine, stateID);
    } while (stateID[0] != EXIT && !shouldWindowClose(engine.window));

    cleanup(engine);

    return 0;
}
