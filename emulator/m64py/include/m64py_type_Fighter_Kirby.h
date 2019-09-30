#ifndef M64PY_TYPE_FIGHTER_KIRBY_H
#define M64PY_TYPE_FIGHTER_KIRBY_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Kirby
{
    m64py_Fighter super;
} m64py_Kirby;

extern PyTypeObject m64py_KirbyType;

int
m64py_KirbyType_init(void);

#endif /* M64PY_TYPE_FIGHTER_KIRBY_H */
