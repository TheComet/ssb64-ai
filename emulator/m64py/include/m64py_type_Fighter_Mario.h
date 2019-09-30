#ifndef M64PY_TYPE_FIGHTER_MARIO_H
#define M64PY_TYPE_FIGHTER_MARIO_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Mario
{
    m64py_Fighter super;
} m64py_Mario;

extern PyTypeObject m64py_MarioType;

int
m64py_MarioType_init(void);

#endif /* M64PY_TYPE_FIGHTER_MARIO_H */
