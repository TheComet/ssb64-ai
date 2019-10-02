#ifndef M64PY_TYPE_PLUGIN_CUCKED_PLUGIN_H
#define M64PY_TYPE_PLUGIN_CUCKED_PLUGIN_H

#include "m64py_type_Plugin.h"

#define m64py_CuckedPlugin_CheckExact(op) \
    (Py_TYPE(op) == &m64py_CuckedPluginType)

typedef m64p_error (*ptr_PluginCuckedStartup)(m64p_dynlib_handle, void* Context, void (*)(void*,int,const char*), const char*);
typedef ptr_PluginShutdown ptr_PluginCuckedShutdown;

typedef struct m64py_CuckedPlugin
{
    m64py_Plugin super;
    ptr_PluginCuckedStartup PluginStartupCucked;
    ptr_PluginCuckedShutdown PluginShutdownCucked;
} m64py_CuckedPlugin;

extern PyTypeObject m64py_CuckedPluginType;

int
m64py_CuckedPluginType_init(void);

#endif /* M64PY_TYPE_PLUGIN_CUCKED_PLUGIN_H */
