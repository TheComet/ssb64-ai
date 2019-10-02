#include "m64py_type_Plugin_CuckedInputPlugin.h"
#include "m64py_type_Emulator.h"
#include "m64p_common.h"
#include "m64p_plugin.h"
#include "osal_dynamiclib.h"
#include <structmember.h>

/* ------------------------------------------------------------------------- */
static void
CuckedInputPlugin_dealloc(m64py_CuckedInputPlugin* self)
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
CuckedInputPlugin_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64p_error result;
    const char* actual_path_to_plugin;
    m64p_plugin_type plugin_type;
    m64p_dynlib_handle corelib_handle;

    PyObject* py_cucked_path;
    PyObject* py_cucked_args;
    m64py_CuckedInputPlugin* self;

    static const char* cucked_path = "./mupen64plus-input-ai-cuck.so";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "siO&", kwds_names, &actual_path_to_plugin, &plugin_type, long2handle_converter, &corelib_handle))
        return NULL;

    /* We need to modify path_to_plugin to load the cucked plugin instead */
    py_cucked_path = PyUnicode_FromString(cucked_path);
    if (py_cucked_path == NULL)
        return NULL;
    py_cucked_args = PyTuple_New(3);
    if (py_cucked_args == NULL)
    {
        Py_DECREF(py_cucked_path);
        return NULL;
    }

    Py_INCREF(PyTuple_GET_ITEM(args, 1));
    Py_INCREF(PyTuple_GET_ITEM(args, 2));
    PyTuple_SET_ITEM(py_cucked_args, 0, py_cucked_path);
    PyTuple_SET_ITEM(py_cucked_args, 1, PyTuple_GET_ITEM(args, 1));
    PyTuple_SET_ITEM(py_cucked_args, 2, PyTuple_GET_ITEM(args, 2));

    self = (m64py_CuckedInputPlugin*)type->tp_base->tp_new(type, py_cucked_args, kwds);
    Py_DECREF(py_cucked_args);

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
#define REQUIRE_FUNC(name) \
        if ((self->name = osal_dynlib_getproc(self->super.handle, #name)) == NULL) \
        { \
            PyErr_Format(PyExc_RuntimeError, "Failed to load function \"%s\" from plugin \"%s\"", #name, cucked_path); \
            goto get_cucked_proc_failed; \
        }

    REQUIRE_FUNC(PluginStartupCucked);
    if ((result = self->PluginStartupCucked(corelib_handle, NULL, NULL, actual_path_to_plugin)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to start plugin %s: Error code %d", actual_path_to_plugin, result);
        goto get_cucked_proc_failed;
    }

    REQUIRE_FUNC(PluginShutdownCucked);
    REQUIRE_FUNC(GetControllerOverride);
    REQUIRE_FUNC(SetControllerOverride);
    REQUIRE_FUNC(GetControllerButtons);
    REQUIRE_FUNC(SetControllerButtons);

    return (PyObject*)self;

    get_cucked_proc_failed : Py_DECREF(self);
    return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CUCKED_INPUT_PLUGIN_DOC,
"Handles loading the cuck input plugin (intercepts the normal input plugin so)\n"
"the AI can control individual players");
PyTypeObject m64py_CuckedInputPluginType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.CuckedInputPlugin",    /* tp_name */
    sizeof(m64py_CuckedInputPlugin),   /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)CuckedInputPlugin_dealloc,  /* tp_dealloc */
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
    CUCKED_INPUT_PLUGIN_DOC,      /* tp_doc */
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
    CuckedInputPlugin_new,             /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_CuckedInputPluginType_init(void)
{
    m64py_CuckedInputPluginType.tp_base = &m64py_PluginType;
    if (PyType_Ready(&m64py_CuckedInputPluginType) < 0)
        return -1;

    return 0;
}
