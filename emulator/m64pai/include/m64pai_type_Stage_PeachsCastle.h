#ifndef M64PAI_TYPE_STAGE_PEACHS_CASTLE_H
#define M64PAI_TYPE_STAGE_PEACHS_CASTLE_H

#include "m64pai_type_Stage.h"

typedef struct m64pai_PeachsCastle
{
    m64pai_Stage super;
} m64pai_PeachsCastle;

extern PyTypeObject m64pai_PeachsCastleType;

int
m64pai_PeachsCastleType_init(void);

#endif /* M64PAI_TYPE_STAGE_PEACHS_CASTLE_H */
