#ifndef M64PY_TYPE_FIGHTER_FOX_H
#define M64PY_TYPE_FIGHTER_FOX_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Fox
{
    m64py_Fighter super;
} m64py_Fox;

extern PyTypeObject m64py_FoxType;

int
m64py_FoxType_init(void);

#endif /* M64PY_TYPE_FIGHTER_FOX_H */
