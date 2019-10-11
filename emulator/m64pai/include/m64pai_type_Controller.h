#ifndef M64PAI_TYPE_CONTROLLER_H
#define M64PAI_TYPE_CONTROLLER_H

#include "m64pai_type_Emulator.h"

typedef struct m64pai_Controller
{
    PyObject_HEAD
    m64pai_Emulator* emu;
    int player_slot;  /* value from 1-4 */
} m64pai_Controller;

extern PyTypeObject m64pai_ControllerType;

int
m64pai_ControllerType_init(void);

#endif /* M64PAI_TYPE_CONTROLLER_H */
