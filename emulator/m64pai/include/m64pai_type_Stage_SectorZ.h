#ifndef M64PAI_TYPE_STAGE_SECTOR_Z_H
#define M64PAI_TYPE_STAGE_SECTOR_Z_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_SectorZ
{
    m64pai_Stage super;
} m64pai_SectorZ;

extern PyTypeObject m64pai_SectorZType;

int
m64pai_SectorZType_init(void);

#endif /* M64PAI_TYPE_STAGE_SECTOR_Z_H */
