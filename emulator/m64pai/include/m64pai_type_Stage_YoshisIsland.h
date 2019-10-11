#ifndef M64PAI_TYPE_STAGE_YOSHIS_ISLAND_H
#define M64PAI_TYPE_STAGE_YOSHIS_ISLAND_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_YoshisIsland
{
    m64pai_Stage super;
} m64pai_YoshisIsland;

extern PyTypeObject m64pai_YoshisIslandType;

int
m64pai_YoshisIslandType_init(void);

#endif /* M64PAI_TYPE_STAGE_YOSHIS_ISLAND_H */
