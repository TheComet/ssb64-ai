#ifndef M64PAI_TYPE_FIGHTER_FOX_H
#define M64PAI_TYPE_FIGHTER_FOX_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Fox
{
    m64pai_Fighter super;
} m64pai_Fox;

extern PyTypeObject m64pai_FoxType;

int
m64pai_FoxType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_FOX_H */
