#ifndef M64PY_TYPE_STAGE_YOSHIS_ISLAND_H
#define M64PY_TYPE_STAGE_YOSHIS_ISLAND_H

#include "m64py_type_Stage.h"

typedef struct m64py_YoshisIsland
{
    m64py_Stage super;
} m64py_YoshisIsland;

extern PyTypeObject m64py_YoshisIslandType;

int
m64py_YoshisIslandType_init(void);

#endif /* M64PY_TYPE_STAGE_YOSHIS_ISLAND_H */
