#ifndef M64PY_TYPE_EMULATOR_H
#define M64PY_TYPE_EMULATOR_H

#include "Python.h"

typedef struct m64py_Emulator
{
    PyObject_HEAD
} m64py_Emulator;

extern PyTypeObject m64py_EmulatorType;

int
m64py_EmulatorType_init(void);

#endif /* M64PY_TYPE_EMULATOR_H */
