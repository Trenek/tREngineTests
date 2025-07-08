#include "engineCore.h"

#include "state.h"

int main() {
    struct EngineCore engine = setup("Engine Tester", "icon/iconka.png");
    void (* const state[])(struct EngineCore *engine, enum state *state) = {
        [TEST] = test,
        [LOAD_TEST] = loadTest,
        [LOAD_RESOURCES] = loadResources,
        [MOVE_NEXT] = moveNext,
    };
    enum state stateID = LOAD_RESOURCES;

    do {
        state[stateID](&engine, &stateID);
    } while (stateID != EXIT && !shouldWindowClose(engine.window));

    cleanup(engine);

    return 0;
}
