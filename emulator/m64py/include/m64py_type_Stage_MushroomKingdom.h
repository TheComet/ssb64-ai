#ifndef M64PY_TYPE_STAGE_MUSHROOM_KINGDOM_H
#define M64PY_TYPE_STAGE_MUSHROOM_KINGDOM_H

#include "m64py_type_Stage.h"

typedef struct m64py_MushroomKingdom
{
    m64py_Stage super;
} m64py_MushroomKingdom;

extern PyTypeObject m64py_MushroomKingdomType;

int
m64py_MushroomKingdomType_init(void);

#endif /* M64PY_TYPE_STAGE_MUSHROOM_KINGDOM_H */
