#ifndef M64PAI_TYPE_FIGHTER_LINK_H
#define M64PAI_TYPE_FIGHTER_LINK_H

#include "m64pai_type_Fighter.h"

typedef struct m64pai_Link
{
    m64pai_Fighter super;
} m64pai_Link;

extern PyTypeObject m64pai_LinkType;

int
m64pai_LinkType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_LINK_H */
