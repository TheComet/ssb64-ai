#ifndef M64PAI_TYPE_FIGHTER_DONKEY_KONG_H
#define M64PAI_TYPE_FIGHTER_DONKEY_KONG_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_DonkeyKong
{
    m64pai_Fighter super;
} m64pai_DonkeyKong;

extern PyTypeObject m64pai_DonkeyKongType;

int
m64pai_DonkeyKongType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_DONKEY_KONG_H */
