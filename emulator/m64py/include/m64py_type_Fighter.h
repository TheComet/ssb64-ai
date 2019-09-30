#ifndef M64PY_TYPE_FIGHTER_H
#define M64PY_TYPE_FIGHTER_H

#include "Python.h"
#include "m64py_type_SSB64.h"

typedef struct m64py_Fighter
{
    PyObject_HEAD
    m64py_SSB64* ssb64;
    int memory_index;
    int player_slot;
    uint32_t n64_memory_address;
} m64py_Fighter;

extern PyTypeObject m64py_FighterType;

int
m64py_FighterType_init(void);

#endif /* M64PY_TYPE_FIGHTER_H */
