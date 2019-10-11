#ifndef M64PAI_TYPE_FIGHTER_YOSHI_H
#define M64PAI_TYPE_FIGHTER_YOSHI_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Yoshi
{
    m64pai_Fighter super;
} m64pai_Yoshi;

extern PyTypeObject m64pai_YoshiType;

int
m64pai_YoshiType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_YOSHI_H */
