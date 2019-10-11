#ifndef M64PAI_TYPE_FIGHTER_H
#define M64PAI_TYPE_FIGHTER_H

#include "Python.h"
#include "m64pai_type_SSB64.h"
#include "m64pai_type_Controller.h"

typedef struct m64pai_Fighter
{
    PyObject_HEAD
    m64pai_SSB64* ssb64;
    m64pai_Controller* controller;
    int memory_index;
    int player_slot;
    uint32_t n64_memory_address;
} m64pai_Fighter;

extern PyTypeObject m64pai_FighterType;

int
m64pai_FighterType_init(void);

#endif /* M64PAI_TYPE_FIGHTER_H */
