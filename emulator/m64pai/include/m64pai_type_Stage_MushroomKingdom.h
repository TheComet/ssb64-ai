#ifndef M64PAI_TYPE_STAGE_MUSHROOM_KINGDOM_H
#define M64PAI_TYPE_STAGE_MUSHROOM_KINGDOM_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_MushroomKingdom
{
    m64pai_Stage super;
} m64pai_MushroomKingdom;

extern PyTypeObject m64pai_MushroomKingdomType;

int
m64pai_MushroomKingdomType_init(void);

#endif /* M64PAI_TYPE_STAGE_MUSHROOM_KINGDOM_H */
