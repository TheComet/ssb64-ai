#ifndef M64PY_TYPE_FIGHTER_PIKACHU_H
#define M64PY_TYPE_FIGHTER_PIKACHU_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Pikachu
{
    m64py_Fighter super;
} m64py_Pikachu;

extern PyTypeObject m64py_PikachuType;

int
m64py_PikachuType_init(void);

#endif /* M64PY_TYPE_FIGHTER_PIKACHU_H */
