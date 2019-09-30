#ifndef M64PY_TYPE_FIGHTER_JIGGLYPUFF_H
#define M64PY_TYPE_FIGHTER_JIGGLYPUFF_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Jigglypuff
{
    m64py_Fighter super;
} m64py_Jigglypuff;

extern PyTypeObject m64py_JigglypuffType;

int
m64py_JigglypuffType_init(void);

#endif /* M64PY_TYPE_FIGHTER_JIGGLYPUFF_H */
