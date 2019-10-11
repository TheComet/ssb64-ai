#ifndef M64PAI_TYPE_STAGE_KONGO_JUNGLE_H
#define M64PAI_TYPE_STAGE_KONGO_JUNGLE_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_KongoJungle
{
    m64pai_Stage super;
} m64pai_KongoJungle;

extern PyTypeObject m64pai_KongoJungleType;

int
m64pai_KongoJungleType_init(void);

#endif /* M64PAI_TYPE_STAGE_KONGO_JUNGLE_H */
