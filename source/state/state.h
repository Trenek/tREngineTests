#ifndef STATE_H
#define STATE_H

enum state {
    MOVE_NEXT,
    LOAD_RESOURCES,
    LOAD_TEST,
    TEST,
    EXIT
};

struct EngineCore;

void test(struct EngineCore *engine, enum state *state);
void loadResources(struct EngineCore *engine, enum state *state);
void loadTest(struct EngineCore *engine, enum state *state);
void moveNext(struct EngineCore *engine, enum state *state);

#endif
