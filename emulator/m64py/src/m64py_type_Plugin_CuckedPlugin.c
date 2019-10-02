#include "m64py_type_Plugin_CuckedPlugin.h"
#include "m64py_type_Emulator.h"
#include "m64p_common.h"
#include "osal_dynamiclib.h"
#include <structmember.h>
#include "m64p_plugin.h"

/* ------------------------------------------------------------------------- */
static void
CuckedPlugin_dealloc(m64py_CuckedPlugin* self)
{
    if (self->PluginShutdownCucked)
        self->PluginShutdownCucked();
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "path_to_plugin",
    "type",
    "corelib_handle",
    NULL
};
static int
long2handle_converter(PyObject* o, m64p_dynlib_handle* address)
{
    *address = PyLong_AsVoidPtr(o);
    if (*address == NULL && PyErr_Occurred())
        return 0;
    return 1;
}
static PyObject*
CuckedPlugin_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64p_error result;
    const char* actual_path_to_plugin;
    m64p_plugin_type plugin_type = (m64p_plugin_type)0;
    m64p_dynlib_handle corelib_handle;

    PyObject* cucked_path;
    PyObject* cucked_args;
    m64py_CuckedPlugin* self;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "siO&", kwds_names, &actual_path_to_plugin, &plugin_type, long2handle_converter, &corelib_handle))
        return NULL;

    /* We need to modify path_to_plugin to load the cucked plugin instead */
    cucked_path = PyUnicode_FromString("./mupen64plus-input-ai-cuck.so");
    if (cucked_path == NULL)
        return NULL;
    cucked_args = PyTuple_New(3);
    if (cucked_args == NULL)
    {
        Py_DECREF(cucked_path);
        return NULL;
    }

    Py_INCREF(PyTuple_GET_ITEM(args, 1));
    Py_INCREF(PyTuple_GET_ITEM(args, 2));
    PyTuple_SET_ITEM(cucked_args, 0, cucked_path);
    PyTuple_SET_ITEM(cucked_args, 1, PyTuple_GET_ITEM(args, 1));
    PyTuple_SET_ITEM(cucked_args, 2, PyTuple_GET_ITEM(args, 2));

    self = (m64py_CuckedPlugin*)type->tp_base->tp_new(type, cucked_args, kwds);
    Py_DECREF(cucked_args);

    if (self == NULL)
        return NULL;

    /*
     * The cucked plugin has custom startup/shutdown functions that let us
     * pass in the name of the actual input library to be loaded. The mupen64plus
     * compliant PluginStartup and PluginShutdown will always return success
     * and do nothing. This is just so the emulator doesn't complain.
     *
     * We could overwrite self->super.PluginStartup and self->super.PluginShutdown,
     * however, this proved to be harder to implement.
     */
    self->PluginStartupCucked  = osal_dynlib_getproc(self->super.handle, "PluginStartupCucked");
    self->PluginShutdownCucked = osal_dynlib_getproc(self->super.handle, "PluginShutdownCucked");
    if (self->PluginStartupCucked == NULL || self->PluginShutdownCucked == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to load function \"PluginStartupCucked\" and \"PluginShutdownCucked\" from plugin \"%s\"", cucked_path); \
        goto get_cucked_proc_failed;
    }

    if ((result = self->PluginStartupCucked(corelib_handle, NULL, NULL, actual_path_to_plugin)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to start plugin %s: Error code %d", actual_path_to_plugin, result);
        goto startup_failed;
    }

    return (PyObject*)self;

    startup_failed         :
    get_cucked_proc_failed : Py_DECREF(self);
    return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CUCKED_PLUGIN_DOC,
"Handles loading the cuck input plugin (intercepts the normal input plugin so)\n"
"the AI can control individual players");
PyTypeObject m64py_CuckedPluginType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.CuckedPlugin",         /* tp_name */
    sizeof(m64py_CuckedPlugin),   /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)CuckedPlugin_dealloc,  /* tp_dealloc */
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
    CUCKED_PLUGIN_DOC,            /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    &m64py_PluginType,            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    CuckedPlugin_new,             /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_CuckedPluginType_init(void)
{
    if (PyType_Ready(&m64py_CuckedPluginType) < 0)
        return -1;

    return 0;
}
