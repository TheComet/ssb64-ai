#ifndef M64PY_TYPE_FIGHTER_CAPTAIN_FALCON_H
#define M64PY_TYPE_FIGHTER_CAPTAIN_FALCON_H

#include "m64py_type_Fighter.h"

typedef struct m64py_CaptainFalcon
{
    m64py_Fighter super;
} m64py_CaptainFalcon;

extern PyTypeObject m64py_CaptainFalconType;

int
m64py_CaptainFalconType_init(void);

#endif /* M64PY_TYPE_FIGHTER_CAPTAIN_FALCON_H */
