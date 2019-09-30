#ifndef M64PY_TYPE_FIGHTER_YOSHI_H
#define M64PY_TYPE_FIGHTER_YOSHI_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Yoshi
{
    m64py_Fighter super;
} m64py_Yoshi;

extern PyTypeObject m64py_YoshiType;

int
m64py_YoshiType_init(void);

#endif /* M64PY_TYPE_FIGHTER_YOSHI_H */
