#ifndef M64PY_TYPE_PLUGIN_H
#define M64PY_TYPE_PLUGIN_H

#include "Python.h"
#include "m64p_types.h"

typedef struct m64py_Emulator m64py_Emulator;

typedef struct m64py_Plugin
{
    PyObject_HEAD
    m64p_plugin_type type;
    m64p_dynlib_handle handle;
} m64py_Plugin;

extern PyTypeObject m64py_PluginType;

int
m64py_PluginType_init(void);

m64py_Plugin*
m64py_Plugin_load(m64py_Emulator* emu, const char* path_to_plugin, m64p_plugin_type type);

#endif /* M64PY_TYPE_PLUGIN_H */
