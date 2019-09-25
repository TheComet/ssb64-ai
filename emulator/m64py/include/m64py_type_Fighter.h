#ifndef M64PY_TYPE_FIGHTER_H
#define M64PY_TYPE_FIGHTER_H

#include "Python.h"
#include "m64py_type_Emulator.h"

typedef struct m64py_Fighter
{
    PyObject_HEAD
    m64py_Emulator* emu;
} m64py_Fighter;

extern PyTypeObject m64py_FighterType;

int
m64py_FighterType_init(void);

#endif /* M64PY_TYPE_FIGHTER_H */
