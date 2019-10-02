#ifndef M64PY_TYPE_PLUGIN_CUCKED_INPUT_PLUGIN_H
#define M64PY_TYPE_PLUGIN_CUCKED_INPUT_PLUGIN_H

#include "m64py_type_Plugin.h"
#include "m64p_plugin.h"

#define m64py_CuckedInputPlugin_CheckExact(op) \
    (Py_TYPE(op) == &m64py_CuckedInputPluginType)

typedef m64p_error (*ptr_PluginCuckedStartup)(m64p_dynlib_handle, void* Context, void (*)(void*,int,const char*), const char*);
typedef int (*ptr_GetControllerOverride)(int ControllerIdx);
typedef void (*ptr_SetControllerOverride)(int ControllerIdx, int enable);
typedef BUTTONS (*ptr_GetControllerButtons)(int ControllerIdx);
typedef void (*ptr_SetControllerButtons)(int ControllerIdx, BUTTONS buttons);
typedef ptr_PluginShutdown ptr_PluginCuckedShutdown;

typedef struct m64py_CuckedInputPlugin
{
    m64py_Plugin super;
    ptr_PluginCuckedStartup   PluginStartupCucked;
    ptr_PluginCuckedShutdown  PluginShutdownCucked;
    ptr_SetControllerOverride SetControllerOverride;
    ptr_GetControllerOverride GetControllerOverride;
    ptr_SetControllerButtons  SetControllerButtons;
    ptr_GetControllerButtons  GetControllerButtons;
} m64py_CuckedInputPlugin;

extern PyTypeObject m64py_CuckedInputPluginType;

int
m64py_CuckedInputPluginType_init(void);

#endif /* M64PY_TYPE_PLUGIN_CUCKED_INPUT_PLUGIN_H */
