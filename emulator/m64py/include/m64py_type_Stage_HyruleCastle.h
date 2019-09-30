#ifndef M64PY_TYPE_STAGE_HYRULE_CASTLE_H
#define M64PY_TYPE_STAGE_HYRULE_CASTLE_H

#include "m64py_type_Stage.h"

typedef struct m64py_HyruleCastle
{
    m64py_Stage super;
} m64py_HyruleCastle;

extern PyTypeObject m64py_HyruleCastleType;

int
m64py_HyruleCastleType_init(void);

#endif /* M64PY_TYPE_STAGE_HYRULE_CASTLE_H */
