#ifndef M64PAI_TYPE_FIGHTER_SAMUS_H
#define M64PAI_TYPE_FIGHTER_SAMUS_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Samus
{
    m64pai_Fighter super;
} m64pai_Samus;

extern PyTypeObject m64pai_SamusType;

int
m64pai_SamusType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_SAMUS_H */
