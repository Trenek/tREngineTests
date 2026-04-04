#ifndef STATE_H
#define STATE_H

enum state {
    OBJ = 0,
    MULTI_OBJ,
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

#endif
