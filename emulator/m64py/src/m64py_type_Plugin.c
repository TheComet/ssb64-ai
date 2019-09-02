#include "m64py_type_Plugin.h"
#include "m64py_type_Emulator.h"
#include "m64p_common.h"
#include "osal_dynamiclib.h"
#include <structmember.h>

/* ------------------------------------------------------------------------- */
static void
Plugin_dealloc(m64py_Plugin* self)
{
    osal_dynlib_close(self->handle);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
PyTypeObject m64py_PluginType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Plugin",               /* tp_name */
    sizeof(m64py_Plugin),         /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Plugin_dealloc,   /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_reserved */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash  */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
    "Plugin objects",             /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    0,                            /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_PluginType_init(void)
{
    m64py_PluginType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&m64py_PluginType) < 0)
        return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
m64py_Plugin*
m64py_Plugin_load(const char* path_to_plugin, m64p_plugin_type type, m64p_dynlib_handle corelib_handle)
{
    m64p_plugin_type PluginType = (m64p_plugin_type)0;
    int PluginVersion = 0;
    const char *PluginName = NULL;

    m64py_Plugin* plugin = (m64py_Plugin*)PyObject_CallObject((PyObject*)&m64py_PluginType, NULL);
    if (plugin == NULL)
        goto alloc_plugin_failed;
    plugin->type = type;

    /* Try to open shared library */
    if (osal_dynlib_open(&plugin->handle, path_to_plugin) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to open shared library \"%s\"", path_to_plugin);
        goto open_lib_failed;
    }

    /* Load function pointers */
#define X(name) \
        if ((plugin->name = osal_dynlib_getproc(plugin->handle, #name)) == NULL) \
        { \
            PyErr_Format(PyExc_RuntimeError, "Failed to load function \"%s\" from plugin \"%s\"", #name, path_to_plugin); \
            goto load_functions_failed; \
        }
    M64PY_PLUGIN_FUNCTIONS
#undef X

    plugin->PluginGetVersion(&PluginType, &PluginVersion, NULL, &plugin->name, NULL);
    if (PluginType != type)
    {
        PyErr_Format(PyExc_RuntimeError, "Shared library \"%s\" is of type %d, but we expected %d instead.", path_to_plugin, PluginType, type);
        goto version_mismatch;
    }

    /* the front-end doesn't talk to the plugins, so we don't care about the plugin version or api version */

    return plugin;

    version_mismatch      :
    load_functions_failed : osal_dynlib_close(plugin->handle);
    open_lib_failed       : Py_DECREF(plugin);
    alloc_plugin_failed   : return NULL;
}
