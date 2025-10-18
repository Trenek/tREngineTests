#ifndef STATE_H
#define STATE_H

enum state {
    MESH = 0,
    FONT,

    MOVE_NEXT = 0,
    MOVE_NEXT_STRING,
    LOAD_RESOURCES,
    LOAD_TEST,
    TEST,
    STATE_Q,

    EXIT
};

struct EngineCore;

void meshTest(struct EngineCore *engine, enum state *state);
void loadMeshResources(struct EngineCore *engine, enum state *state);
void loadMeshTest(struct EngineCore *engine, enum state *state);
void moveNextMesh(struct EngineCore *engine, enum state *state);

void fontTest(struct EngineCore *engine, enum state *state);
void loadFontResources(struct EngineCore *engine, enum state *state);
void loadFontTest(struct EngineCore *engine, enum state *state);
void moveNextFont(struct EngineCore *engine, enum state *state);
void moveNextString(struct EngineCore *engine, enum state *state);

#endif
