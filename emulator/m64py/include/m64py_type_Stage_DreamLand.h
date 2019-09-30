#ifndef M64PY_TYPE_STAGE_DREAM_LAND_H
#define M64PY_TYPE_STAGE_DREAM_LAND_H

#include "m64py_type_Stage.h"

typedef struct m64py_DreamLand
{
    m64py_Stage super;
} m64py_DreamLand;

extern PyTypeObject m64py_DreamLandType;

int
m64py_DreamLandType_init(void);

#endif /* M64PY_TYPE_STAGE_DREAM_LAND_H */
