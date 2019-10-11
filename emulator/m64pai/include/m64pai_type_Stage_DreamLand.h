#ifndef M64PAI_TYPE_STAGE_DREAM_LAND_H
#define M64PAI_TYPE_STAGE_DREAM_LAND_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_DreamLand
{
    m64pai_Stage super;
} m64pai_DreamLand;

extern PyTypeObject m64pai_DreamLandType;

int
m64pai_DreamLandType_init(void);

#endif /* M64PAI_TYPE_STAGE_DREAM_LAND_H */
