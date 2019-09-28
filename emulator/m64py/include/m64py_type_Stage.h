#ifndef M64PY_TYPE_STAGE_H
#define M64PY_TYPE_STAGE_H

#include "Python.h"
#include "m64py_type_SSB64.h"

typedef struct m64py_Stage
{
    PyObject_HEAD
    m64py_SSB64* ssb64;
} m64py_Stage;

extern PyTypeObject m64py_StageType;

int
m64py_StageType_init(void);

#endif /* M64PY_TYPE_STAGE_H */
