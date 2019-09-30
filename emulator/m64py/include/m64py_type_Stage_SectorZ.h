#ifndef M64PY_TYPE_STAGE_SECTOR_Z_H
#define M64PY_TYPE_STAGE_SECTOR_Z_H

#include "m64py_type_Stage.h"

typedef struct m64py_SectorZ
{
    m64py_Stage super;
} m64py_SectorZ;

extern PyTypeObject m64py_SectorZType;

int
m64py_SectorZType_init(void);

#endif /* M64PY_TYPE_STAGE_SECTOR_Z_H */
