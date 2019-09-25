#ifndef M64PY_TYPE_SSB64_H
#define M64PY_TYPE_SSB64_H

#include "m64py_type_Emulator.h"

typedef struct m64py_memory_interface_t m64py_memory_interface_t;

typedef struct m64py_SSB64
{
    PyObject_HEAD
    m64py_Emulator* emu;
    m64py_memory_interface_t* mem_iface;
} m64py_SSB64;

extern PyTypeObject m64py_SSB64Type;

int
m64py_SSB64Type_init(void);

#endif /* M64PY_TYPE_SSB64_H */
