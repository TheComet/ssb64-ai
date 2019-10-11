#ifndef M64PAI_TYPE_SSB64_H
#define M64PAI_TYPE_SSB64_H

#include "m64pai_type_Emulator.h"

typedef struct m64pai_memory_interface_t m64pai_memory_interface_t;

typedef struct m64pai_SSB64
{
    PyObject_HEAD
    m64pai_Emulator* emu;
    m64pai_memory_interface_t* memory_interface;
} m64pai_SSB64;

extern PyTypeObject m64pai_SSB64Type;

int
m64pai_SSB64Type_init(void);

#endif /* M64PAI_TYPE_SSB64_H */
