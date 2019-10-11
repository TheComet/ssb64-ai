#ifndef M64PAI_TYPE_FIGHTER_CAPTAIN_FALCON_H
#define M64PAI_TYPE_FIGHTER_CAPTAIN_FALCON_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_CaptainFalcon
{
    m64pai_Fighter super;
} m64pai_CaptainFalcon;

extern PyTypeObject m64pai_CaptainFalconType;

int
m64pai_CaptainFalconType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_CAPTAIN_FALCON_H */
