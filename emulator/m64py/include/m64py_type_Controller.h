#ifndef M64PY_TYPE_CONTROLLER_H
#define M64PY_TYPE_CONTROLLER_H

#include "m64py_type_Emulator.h"

typedef struct m64py_Controller
{
    PyObject_HEAD
    m64py_Emulator* emu;
    int player_slot;  /* value from 1-4 */
} m64py_Controller;

extern PyTypeObject m64py_ControllerType;

int
m64py_ControllerType_init(void);

#endif /* M64PY_TYPE_CONTROLLER_H */
