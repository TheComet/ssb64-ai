#ifndef M64PAI_TYPE_FIGHTER_JIGGLYPUFF_H
#define M64PAI_TYPE_FIGHTER_JIGGLYPUFF_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Jigglypuff
{
    m64pai_Fighter super;
} m64pai_Jigglypuff;

extern PyTypeObject m64pai_JigglypuffType;

int
m64pai_JigglypuffType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_JIGGLYPUFF_H */
