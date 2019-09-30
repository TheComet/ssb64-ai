#ifndef M64PY_TYPE_STAGE_SAFFRON_CITY_H
#define M64PY_TYPE_STAGE_SAFFRON_CITY_H

#include "m64py_type_Stage.h"

typedef struct m64py_SaffronCity
{
    m64py_Stage super;
} m64py_SaffronCity;

extern PyTypeObject m64py_SaffronCityType;

int
m64py_SaffronCityType_init(void);

#endif /* M64PY_TYPE_STAGE_SAFFRON_CITY_H */
