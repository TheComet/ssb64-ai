#ifndef M64PY_TYPE_STAGE_PLANET_ZEBES_H
#define M64PY_TYPE_STAGE_PLANET_ZEBES_H

#include "m64py_type_Stage.h"

typedef struct m64py_PlanetZebes
{
    m64py_Stage super;
} m64py_PlanetZebes;

extern PyTypeObject m64py_PlanetZebesType;

int
m64py_PlanetZebesType_init(void);

#endif /* M64PY_TYPE_STAGE_PLANET_ZEBES_H */
