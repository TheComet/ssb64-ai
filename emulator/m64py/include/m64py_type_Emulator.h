#ifndef M64PY_TYPE_EMULATOR_H
#define M64PY_TYPE_EMULATOR_H

#include "Python.h"
#include "osal_dynamiclib.h"

#define m64py_EmulatorType_CheckExact(op) \
    (Py_TYPE(op) == &m64py_EmulatorType)

typedef struct m64py_Emulator
{
    PyObject_HEAD
    m64p_dynlib_handle corelib_handle;
    PyObject* input_plugin;
    PyObject* audio_plugin;
    PyObject* video_plugin;
    PyObject* rsp_plugin;
} m64py_Emulator;

extern PyTypeObject m64py_EmulatorType;

int
m64py_EmulatorType_init(void);

#endif /* M64PY_TYPE_EMULATOR_H */
