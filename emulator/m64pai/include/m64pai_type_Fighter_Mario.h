#ifndef M64PAI_TYPE_FIGHTER_MARIO_H
#define M64PAI_TYPE_FIGHTER_MARIO_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Mario
{
    m64pai_Fighter super;
} m64pai_Mario;

extern PyTypeObject m64pai_MarioType;

int
m64pai_MarioType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_MARIO_H */
