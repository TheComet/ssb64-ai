#ifndef M64PAI_TYPE_FIGHTER_PIKACHU_H
#define M64PAI_TYPE_FIGHTER_PIKACHU_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Pikachu
{
    m64pai_Fighter super;
} m64pai_Pikachu;

extern PyTypeObject m64pai_PikachuType;

int
m64pai_PikachuType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_PIKACHU_H */
