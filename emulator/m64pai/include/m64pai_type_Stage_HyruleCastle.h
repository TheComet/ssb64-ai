#ifndef M64PAI_TYPE_STAGE_HYRULE_CASTLE_H
#define M64PAI_TYPE_STAGE_HYRULE_CASTLE_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_HyruleCastle
{
    m64pai_Stage super;
} m64pai_HyruleCastle;

extern PyTypeObject m64pai_HyruleCastleType;

int
m64pai_HyruleCastleType_init(void);

#endif /* M64PAI_TYPE_STAGE_HYRULE_CASTLE_H */
