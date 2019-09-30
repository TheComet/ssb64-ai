#ifndef M64PY_TYPE_FIGHTER_LINK_H
#define M64PY_TYPE_FIGHTER_LINK_H

#include "m64py_type_Fighter.h"

typedef struct m64py_Link
{
    m64py_Fighter super;
} m64py_Link;

extern PyTypeObject m64py_LinkType;

int
m64py_LinkType_init(void);

#endif /* M64PY_TYPE_FIGHTER_LINK_H */
