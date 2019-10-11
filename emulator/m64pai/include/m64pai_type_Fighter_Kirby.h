#ifndef M64PAI_TYPE_FIGHTER_KIRBY_H
#define M64PAI_TYPE_FIGHTER_KIRBY_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Kirby
{
    m64pai_Fighter super;
} m64pai_Kirby;

extern PyTypeObject m64pai_KirbyType;

int
m64pai_KirbyType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_KIRBY_H */
