#ifndef M64PY_TYPE_FIGHTER_SAMUS_H
#define M64PY_TYPE_FIGHTER_SAMUS_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Samus
{
    m64py_Fighter super;
} m64py_Samus;

extern PyTypeObject m64py_SamusType;

int
m64py_SamusType_init(void);

#endif /* M64PY_TYPE_FIGHTER_SAMUS_H */
