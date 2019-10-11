#ifndef M64PAI_TYPE_STAGE_PLANET_ZEBES_H
#define M64PAI_TYPE_STAGE_PLANET_ZEBES_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_PlanetZebes
{
    m64pai_Stage super;
} m64pai_PlanetZebes;

extern PyTypeObject m64pai_PlanetZebesType;

int
m64pai_PlanetZebesType_init(void);

#endif /* M64PAI_TYPE_STAGE_PLANET_ZEBES_H */
