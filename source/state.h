#ifndef STATE_H
#define STATE_H

enum state {
    OBJ = 0,
    MULTI_OBJ,
    FONT,
    GLTF,
    REC,

    MOVE_NEXT = 0,
    MOVE_NEXT_STRING,
    LOAD_RESOURCES,
    LOAD_TEST,
    TEST,
    STATE_Q,

    EXIT
};

struct EngineCore;

void objTest(struct EngineCore *engine, enum state *state);
void loadObjResources(struct EngineCore *engine, enum state *state);
void loadObjTest(struct EngineCore *engine, enum state *state);
void moveNextObj(struct EngineCore *engine, enum state *state);

void multiObjTest(struct EngineCore *engine, enum state *state);
void loadMultiObjResources(struct EngineCore *engine, enum state *state);
void loadMultiObjTest(struct EngineCore *engine, enum state *state);
void moveNextMultiObj(struct EngineCore *engine, enum state *state);

void fontTest(struct EngineCore *engine, enum state *state);
void loadFontResources(struct EngineCore *engine, enum state *state);
void loadFontTest(struct EngineCore *engine, enum state *state);
void moveNextFont(struct EngineCore *engine, enum state *state);
void moveNextString(struct EngineCore *engine, enum state *state);

void gltfTest(struct EngineCore *engine, enum state *state);
void loadGltfResources(struct EngineCore *engine, enum state *state);
void loadGltfTest(struct EngineCore *engine, enum state *state);
void moveNextGltf(struct EngineCore *engine, enum state *state);

void recTest(struct EngineCore *engine, enum state *state);
void loadRecResources(struct EngineCore *engine, enum state *state);
void loadRecTest(struct EngineCore *engine, enum state *state);
#endif
