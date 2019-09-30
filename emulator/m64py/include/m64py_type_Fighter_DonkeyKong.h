#ifndef M64PY_TYPE_FIGHTER_DONKEY_KONG_H
#define M64PY_TYPE_FIGHTER_DONKEY_KONG_H

#include "m64py_type_Fighter.h"

typedef struct m64py_DonkeyKong
{
    m64py_Fighter super;
} m64py_DonkeyKong;

extern PyTypeObject m64py_DonkeyKongType;

int
m64py_DonkeyKongType_init(void);

#endif /* M64PY_TYPE_FIGHTER_DONKEY_KONG_H */
