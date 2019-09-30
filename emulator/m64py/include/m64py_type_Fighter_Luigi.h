#ifndef M64PY_TYPE_FIGHTER_LUIGI_H
#define M64PY_TYPE_FIGHTER_LUIGI_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Luigi
{
    m64py_Fighter super;
} m64py_Luigi;

extern PyTypeObject m64py_LuigiType;

int
m64py_LuigiType_init(void);

#endif /* M64PY_TYPE_FIGHTER_LUIGI_H */
