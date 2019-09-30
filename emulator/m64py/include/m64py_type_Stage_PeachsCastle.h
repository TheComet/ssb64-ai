#ifndef M64PY_TYPE_STAGE_PEACHS_CASTLE_H
#define M64PY_TYPE_STAGE_PEACHS_CASTLE_H

#include "m64py_type_Stage.h"

typedef struct m64py_PeachsCastle
{
    m64py_Stage super;
} m64py_PeachsCastle;

extern PyTypeObject m64py_PeachsCastleType;

int
m64py_PeachsCastleType_init(void);

#endif /* M64PY_TYPE_STAGE_PEACHS_CASTLE_H */
