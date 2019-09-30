#ifndef M64PY_TYPE_FIGHTER_NESS_H
#define M64PY_TYPE_FIGHTER_NESS_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Ness
{
    m64py_Fighter super;
} m64py_Ness;

extern PyTypeObject m64py_NessType;

int
m64py_NessType_init(void);

#endif /* M64PY_TYPE_FIGHTER_NESS_H */
