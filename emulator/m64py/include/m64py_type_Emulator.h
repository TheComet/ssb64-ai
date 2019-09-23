#ifndef M64PY_TYPE_EMULATOR_H
#define M64PY_TYPE_EMULATOR_H

#include "Python.h"
#include "osal_dynamiclib.h"
#include "m64p_frontend.h"
#include "m64p_debugger.h"

#define m64py_EmulatorType_CheckExact(op) \
    (Py_TYPE(op) == &m64py_EmulatorType)

#define M64PY_CORELIB_FUNCTIONS \
    X(CoreStartup) \
    X(CoreShutdown) \
    X(CoreAttachPlugin) \
    X(CoreDetachPlugin) \
    X(CoreDoCommand) \
    X(DebugMemRead32)

typedef struct m64py_Emulator_corelib_interface
{
    m64p_dynlib_handle handle;
#define X(name) ptr_##name name;
    M64PY_CORELIB_FUNCTIONS
#undef X
} m64py_corelib_interface;

typedef struct m64py_Emulator
{
    PyObject_HEAD
    m64py_corelib_interface corelib;
    PyObject* input_plugin;
    PyObject* audio_plugin;
    PyObject* video_plugin;
    PyObject* rsp_plugin;
    PyObject* frame_callback;
    PyObject* log_message_callback;
    char is_rom_loaded;
} m64py_Emulator;

extern PyTypeObject m64py_EmulatorType;

int
m64py_EmulatorType_init(void);

int
m64py_Emulator_start_plugins(m64py_Emulator* self);

void
m64py_Emulator_stop_plugins(m64py_Emulator* self);

#endif /* M64PY_TYPE_EMULATOR_H */
