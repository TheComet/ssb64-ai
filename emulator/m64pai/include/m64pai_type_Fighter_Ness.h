#ifndef M64PAI_TYPE_FIGHTER_NESS_H
#define M64PAI_TYPE_FIGHTER_NESS_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Ness
{
    m64pai_Fighter super;
} m64pai_Ness;

extern PyTypeObject m64pai_NessType;

int
m64pai_NessType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_NESS_H */
