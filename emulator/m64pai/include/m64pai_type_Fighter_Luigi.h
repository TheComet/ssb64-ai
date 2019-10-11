#ifndef M64PAI_TYPE_FIGHTER_LUIGI_H
#define M64PAI_TYPE_FIGHTER_LUIGI_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Luigi
{
    m64pai_Fighter super;
} m64pai_Luigi;

extern PyTypeObject m64pai_LuigiType;

int
m64pai_LuigiType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_LUIGI_H */
