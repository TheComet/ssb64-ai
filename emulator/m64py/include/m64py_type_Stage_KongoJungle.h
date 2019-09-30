#ifndef M64PY_TYPE_STAGE_KONGO_JUNGLE_H
#define M64PY_TYPE_STAGE_KONGO_JUNGLE_H

#include "m64py_type_Stage.h"

typedef struct m64py_KongoJungle
{
    m64py_Stage super;
} m64py_KongoJungle;

extern PyTypeObject m64py_KongoJungleType;

int
m64py_KongoJungleType_init(void);

#endif /* M64PY_TYPE_STAGE_KONGO_JUNGLE_H */
