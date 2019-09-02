#ifndef M64PY_TYPE_PLUGIN_H
#define M64PY_TYPE_PLUGIN_H

#include "Python.h"
#include "m64p_common.h"

#define M64PY_PLUGIN_FUNCTIONS \
    X(PluginGetVersion) \
    X(PluginStartup) \
    X(PluginShutdown)

#define m64py_Plugin_CheckExact(op) \
    (Py_TYPE(op) == &m64py_PluginType)

typedef struct m64py_Emulator m64py_Emulator;

typedef struct m64py_Plugin
{
    PyObject_HEAD
    m64p_plugin_type type;
    m64p_dynlib_handle handle;
    const char* name;
#define X(name) ptr_##name name;
    M64PY_PLUGIN_FUNCTIONS
#undef X
} m64py_Plugin;

extern PyTypeObject m64py_PluginType;

int
m64py_PluginType_init(void);

m64py_Plugin*
m64py_Plugin_load(const char* path_to_plugin, m64p_plugin_type type, m64p_dynlib_handle corelib_handle);

#endif /* M64PY_TYPE_PLUGIN_H */
