#ifndef M64PAI_TYPE_EMULATOR_H
#define M64PAI_TYPE_EMULATOR_H

#include "Python.h"
#include "osal_dynamiclib.h"
#include "m64p_frontend.h"
#include "m64p_debugger.h"
#include "m64p_ai.h"

#define m64pai_EmulatorType_CheckExact(op) \
    (Py_TYPE(op) == &m64pai_EmulatorType)

#define M64PAI_CORELIB_FUNCTIONS \
    X(CoreStartup) \
    X(CoreShutdown) \
    X(CoreAttachPlugin) \
    X(CoreDetachPlugin) \
    X(CoreDoCommand) \
    X(CoreAddCheat) \
    X(CoreCheatEnabled) \
    X(CoreGetRomSettings) \
    X(AISetVICallback) \
    X(DebugMemRead32) \
    X(DebugMemRead16) \
    X(DebugMemRead8) \
    X(DebugMemWrite32) \
    X(DebugMemWrite16) \
    X(DebugMemWrite8)

typedef struct m64pai_Emulator_corelib_interface
{
    m64p_dynlib_handle handle;
#define X(name) ptr_##name name;
    M64PAI_CORELIB_FUNCTIONS
#undef X
} m64pai_corelib_interface;

typedef struct m64pai_Emulator
{
    PyObject_HEAD
    m64pai_corelib_interface corelib;
    PyObject* input_plugin;
    PyObject* audio_plugin;
    PyObject* video_plugin;
    PyObject* rsp_plugin;
    PyObject* frame_callback;
    PyObject* log_message_callback;
    PyObject* vi_callback;
    char is_rom_loaded;
} m64pai_Emulator;

extern PyTypeObject m64pai_EmulatorType;

int
m64pai_EmulatorType_init(void);

int
m64pai_Emulator_start_plugins(m64pai_Emulator* self);

void
m64pai_Emulator_stop_plugins(m64pai_Emulator* self);

#endif /* M64PAI_TYPE_EMULATOR_H */
