#ifndef M64PAI_TYPE_STAGE_H
#define M64PAI_TYPE_STAGE_H

#include "Python.h"
#include "m64pai_type_SSB64.h"

typedef struct m64pai_Stage
{
    PyObject_HEAD
    m64pai_SSB64* ssb64;
} m64pai_Stage;

extern PyTypeObject m64pai_StageType;

int
m64pai_StageType_init(void);

#endif /* M64PAI_TYPE_STAGE_H */
