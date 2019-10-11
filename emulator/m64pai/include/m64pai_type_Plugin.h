#ifndef M64PAI_TYPE_PLUGIN_H
#define M64PAI_TYPE_PLUGIN_H

#include "Python.h"
#include "m64p_common.h"

#define M64PAI_PLUGIN_FUNCTIONS \
    X(PluginGetVersion) \
    X(PluginStartup) \
    X(PluginShutdown)

#define m64pai_Plugin_CheckExact(op) \
    (Py_TYPE(op) == &m64pai_PluginType)

typedef struct m64pai_Emulator m64pai_Emulator;

typedef struct m64pai_Plugin
{
    PyObject_HEAD

    /*! Whether it's input/video/audio/rsp/etc. */
    m64p_plugin_type type;

    /*! Handle to the loaded shared library */
    m64p_dynlib_handle handle;

    /*!
     * Points to whatever PluginGetVersion() returns, which I think is just a
     * static string inside the loaded shared library. Make sure to NULL this
     * when the plugin is unloaded.
     */
    const char* name;

    /* Function pointers for plugin API */
#define X(name) ptr_##name name;
    M64PAI_PLUGIN_FUNCTIONS
#undef X
} m64pai_Plugin;

extern PyTypeObject m64pai_PluginType;

int
m64pai_PluginType_init(void);

#endif /* M64PAI_TYPE_PLUGIN_H */
