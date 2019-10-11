#ifndef M64PAI_TYPE_STAGE_SAFFRON_CITY_H
#define M64PAI_TYPE_STAGE_SAFFRON_CITY_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_SaffronCity
{
    m64pai_Stage super;
} m64pai_SaffronCity;

extern PyTypeObject m64pai_SaffronCityType;

int
m64pai_SaffronCityType_init(void);

#endif /* M64PAI_TYPE_STAGE_SAFFRON_CITY_H */
